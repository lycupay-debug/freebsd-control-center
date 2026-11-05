# FreeBSD Device-Control Research (FCC)

**Research date:** 2026-11-06 (local laptop date)  
**Target:** FreeBSD 15.1-RELEASE, Dell laptop, Intel HD Graphics 520, Conexant CX20722  
**Scope:** Read-only source and capability inspection. No hardware state was changed.

## 1. Evidence boundary

This document separates API symbols, advertised device capabilities, filesystem permissions, successful hardware writes, read-back verification, and restoration. Only API/source inspection, capability reads, and filesystem permission checks were performed. No brightness, volume, or mute write was issued.

## 2. Backlight path

### 2.1 Target observations

- Device: /dev/backlight/backlight0; alias /dev/backlight/intel_backlight0.
- Name/type from backlight -i: intel_backlight / Panel.
- Current brightness from the existing read-only probe: 9.
- BACKLIGHTGETSTATUS reports nlevels = 0.
- Device node permissions: root:video, mode 0660.
- Normal desktop user lichen belongs to video; read and write permission checks both succeed. This establishes filesystem access only, not successful or safe ioctl writes.

### 2.2 FreeBSD interface findings

The installed /usr/src/sys/sys/backlight.h defines struct backlight_props with brightness, nlevels, and an optional fixed-size levels[] array, plus BACKLIGHTGETSTATUS, BACKLIGHTUPDATESTATUS, and BACKLIGHTGETINFO.

The installed backlight(9) manual states that GET_STATUS supplies current brightness and optional supported levels. A zero nlevels means no discrete level list was supplied; it does not mean the device is broken and does not provide a min/max range.

The installed generic /usr/src/sys/dev/backlight/backlight.c implementation:
- passes GET_STATUS to the device driver's BACKLIGHT_GET_STATUS method;
- passes UPDATE_STATUS to BACKLIGHT_UPDATE_STATUS when the requested value differs from its cached value;
- does not itself establish a universal brightness range or clamp policy.

The installed backlight(8) utility accepts command-line values in the range 0..100 and sends the value through BACKLIGHTUPDATESTATUS. This is a userland utility contract, not sufficient evidence that every driver's hardware mapping or range is 0..100.

The local i915 source inspected for this laptop registers an intel_backlight device through the LinuxKPI backlight interface and sets a driver-side max_brightness from the panel's backlight maximum. The FreeBSD LinuxKPI-to-backlight translation path and actual runtime mapping have not yet been fully traced. Do not infer the FreeBSD ioctl range from the Linux structure alone.

### 2.3 Engineering decision

- Keep the existing FCC brightness adapter read-only.
- Do not populate a synthetic 0..100 range when nlevels == 0.
- Do not send BACKLIGHTUPDATESTATUS until the LinuxKPI translation, target driver's accepted range, and rollback behavior are established.
- A future UI may show the raw current value and an explicit “range unavailable” state. It must not expose a slider whose limits are guessed.
- If the driver does not expose a trustworthy range, a separate explicitly configured policy may be considered only after user confirmation and a safe write/readback/restore test. That policy must not be represented as hardware-reported metadata.

## 3. Mixer and mute path

### 3.1 Target observations

A temporary diagnostic program was compiled under /tmp and opened each mixer with O_RDONLY | O_CLOEXEC. It queried read ioctls only.

| Device | DEVMASK | Master volume | Mute bit advertised |
|---|---:|---:|---|
| /dev/mixer0 | 0x01002831 | 60:60 | no |
| /dev/mixer1 | 0x00002011 | 86:86 | no |
| /dev/mixer2 | 0x00000011 | 100:100 | no |

For all three devices, SOUND_MIXER_READ_MUTE returned success and raw value 0, but SOUND_MASK_MUTE was absent from the corresponding SOUND_MIXER_READ_DEVMASK result. A successful ioctl return does not override the device capability mask. The raw zero must not be reported as “unmuted”; mute is unsupported/not advertised by these devices.

Device nodes /dev/mixer0..2 are world-readable and world-writable on this machine. The normal user passes filesystem read/write permission checks. This does not prove that a device implements or accepts a write control.

### 3.2 FreeBSD OSS API findings

The installed /usr/include/sys/soundcard.h defines SOUND_MIXER_READ_DEVMASK for advertised controls; SOUND_MIXER_READ_VOLUME and SOUND_MIXER_WRITE_VOLUME; and SOUND_MIXER_MUTE, SOUND_MASK_MUTE, SOUND_MIXER_READ_MUTE, and SOUND_MIXER_WRITE_MUTE.

The existence of a MUTE ioctl macro is not evidence that a particular mixer advertises mute. Capability checks must precede interpretation or use of optional controls.

The installed libmixer manual documents mixer_set_mute() and a per-device mute mask, but the target mixers' DEVMASK results do not advertise the MUTE bit. The library API is not a reason to expose a nonfunctional mute button.

### 3.3 Engineering decision

- Continue to expose master-volume read-only data.
- Report mute as unsupported for the currently observed mixer devices; do not translate the raw MUTE read value of zero into an unmuted state.
- Do not issue SOUND_MIXER_WRITE_MUTE on these devices.
- Do not silently emulate mute by setting volume to zero. That is a different operation, can race with other applications, and requires preserving/restoring a potentially changed volume value.
- Volume writes remain unimplemented until a separate single-control test has a saved original value, a confirmed restore command, a bounded adjustment, readback, and restoration readback.

## 4. Security and runtime constraints

- FCC remains a normal-user GUI.
- No privileged helper is justified by the current evidence.
- No device permissions, group memberships, sysctls, driver settings, or persistent startup configuration were changed during this research.
- No NVIDIA/NVDEC, i915, or LinuxKPI code was modified.
- The temporary diagnostic source and executable were placed under /tmp, not added to the repository.

## 5. Next implementation gates

1. Trace the FreeBSD LinuxKPI backlight registration/update bridge used by this installed i915 build and identify the exact accepted value domain.
2. If the value domain is not discoverable from a stable public interface, design an explicit range policy and obtain user approval before any write.
3. For audio, decide whether V1.0 requires true hardware mute or whether mute must be omitted on devices that do not advertise it. Current hardware evidence supports omission.
4. Add fake/injected backend tests for missing devices, permission errors, unsupported controls, malformed ranges, and failed readback before implementing write methods.
5. Only then propose a one-control-at-a-time hardware write experiment with saved original state and verified restoration.

## 6. Commands and test evidence

Read-only checks executed on the target:
- backlight -i
- mixer -a -s
- temporary C diagnostic using SOUND_MIXER_READ_DEVMASK, SOUND_MIXER_READ_MUTE, and SOUND_MIXER_READ_VOLUME, opening mixer nodes read-only;
- source inspection of /usr/src/sys/dev/backlight/backlight.c, /usr/src/sys/sys/backlight.h, /usr/src/usr.bin/backlight/backlight.c, and the local i915 intel_backlight.c;
- filesystem permission checks with test -r and test -w.

No device write test was run. No write behavior is marked verified by this document.
