# FreeBSD Platform Backends (P0)

Status: initial read-only implementation; no device write contract is exposed.

## Responsibilities and dependency direction

- `include/fcc/platform/probe_result.h`: shared diagnostic/result model; standard library only.
- `brightness_backend.h/.cpp`: enumerates `/dev/backlight`, resolves duplicate symlink targets, reads device metadata and current brightness through FreeBSD backlight ioctls.
- `audio_backend.h/.cpp`: enumerates `/dev/mixerN`, checks the OSS device mask and reads master stereo volume using `SOUND_MIXER_READ_VOLUME`.
- `src/app/main.cpp`: composition root; runs both probes in QtConcurrent and renders results in the P0 window.
- `src/platform_probe/main.cpp`: command-line diagnostic for validating platform reads independently of Qt.
- `tests/unit/test_platform_values.cpp`: pure value-decoding tests; does not access hardware.

The platform library has no Qt dependency. The GUI depends on the platform library's public C++ interface; the platform library never imports GUI types.

## Brightness discovery

1. Open `/dev/backlight` and enumerate non-dot entries.
2. Resolve each entry with `realpath` where possible and collapse duplicate aliases.
3. Open each endpoint with `O_RDONLY | O_CLOEXEC`.
4. Read `BACKLIGHTGETINFO` and `BACKLIGHTGETSTATUS`.
5. Reject level counts above `BACKLIGHTMAXLEVELS`; a zero `nlevels` is valid and means the driver did not report a discrete level list.
6. Preserve raw current brightness; report min/max only when the driver supplies levels.

No `BACKLIGHTUPDATESTATUS` call exists in this P0 adapter. The app does not alter brightness. See the FreeBSD [backlight(9) interface](https://man.freebsd.org/cgi/man.cgi?query=backlight&sektion=9) for the optional supported-level semantics.

## Audio discovery

1. Probe bounded mixer unit paths `/dev/mixer0` through `/dev/mixer31`.
2. Open existing nodes with `O_RDONLY | O_CLOEXEC`.
3. Query `SOUND_MIXER_READ_DEVMASK`; require the master-volume bit.
4. Query `SOUND_MIXER_READ_VOLUME`; decode OSS low-byte left and next-byte right values.
5. Reject channel values outside 0..100.
6. Read the descriptive label from `dev.pcm.N.%desc` and compare the unit with `hw.snd.default_unit`.

This first adapter intentionally reports master volume only. It does not claim mute-state support and exposes no write operation. Mute capability and semantics require separate verification before implementation.

## Error model

Each report retains successful devices and per-endpoint issues. Permission failures are distinguished from other I/O errors. Missing endpoints and unsupported controls are explicit; no synthetic volume or brightness values are returned.

## Security and threading

- No root helper, shell command execution, sysctl writes, mixer writes or backlight writes.
- Device nodes are opened read-only.
- QtConcurrent keeps discovery away from the GUI thread.
- The current probe is bounded to one backlight directory and 32 mixer units.

## Verified on the development laptop

Record actual build/test/probe output in the project progress page. A successful read-only query does not prove write capability, write permissions, hotplug handling, or safe restoration.

## Follow-up capability research (2026-11-06)

See FREEBSD_DEVICE_CONTROL_RESEARCH.md for source-level findings and target measurements.

Important target-specific constraints:
- The Intel backlight node is readable and writable by the current user at the filesystem permission layer, but nlevels=0 leaves the hardware range unspecified. The generic FreeBSD backlight layer does not establish a universal range. FCC remains read-only until the i915 LinuxKPI bridge and accepted value domain are traced.
- All three target mixer DEVMASK values omit SOUND_MASK_MUTE. Although READ_MUTE returns zero, that raw result is not a valid mute-state indication when the capability bit is absent. FCC must report mute unsupported on these devices.
- No write ioctl was issued. Filesystem write permission is not proof of successful or safe device control.


## Explicit optional-control capability reporting

The audio adapter preserves whether SOUND_MASK_MUTE is present in each mixer's SOUND_MIXER_READ_DEVMASK. A raw SOUND_MIXER_READ_MUTE result is not queried or interpreted as state when the capability bit is absent. The P0 window now reports mute as unsupported for devices that do not advertise it; it does not emulate mute by setting volume to zero.
