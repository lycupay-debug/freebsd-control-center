# Security and Privilege Model

## Trust boundary
FCC runs as logged-in user. GUI must not run as root. UI controls are not an authorization boundary.

## Device access
P0 inspects sysctl/backlight/mixer/device-node permissions first. Record effective user, group, mode and access results. Do not change permissions or add groups as an unreviewed workaround.

If privilege is proven necessary, pause and design a narrow helper: operation allowlist; strict argument/range validation; fixed device allowlist; no arbitrary command execution; authenticated local IPC and peer checks; bounded request size/time/rate; auditable outcomes; separate tests and uninstall plan. Whole GUI as root is unacceptable. The current audio backend master-volume setter remains normal-user only and enforces a canonical `/dev/mixerN` path, DEVMASK capability, 0..100 bounds, original-value capture, readback and restoration attempt. The GUI exposes it only through a user-activated Apply action; moving the slider alone does not write hardware.

## Input/config/logs
Treat UI values, device IDs, D-Bus messages and config as untrusted. Validate at service/backend boundary. Prefer native APIs over shell construction. Store preferences only, never credentials/tokens. Bound and rotate logs; never log secrets or unbounded device output.

## Supply chain
Use signed FreeBSD packages; record versions/origins; review maintenance, vulnerabilities, license and size. Avoid unverified scripts/mirrors.

## Release checklist
- [ ] Normal-user execution
- [ ] No unnecessary setuid/setgid
- [ ] No secrets in repo/history/logs
- [ ] Explicit validation and bounded operations
- [ ] Permission-denied path tested
- [x] Audio write requires explicit user Apply and performs readback/restoration attempt
- [ ] Injected write/readback failure and failed-restoration paths tested
- [ ] Any helper separately designed/reviewed
- [ ] Uninstall removes only FCC-owned files
