# sensors_read

Telemetry publisher for Navio2-based sensor readings, with supporting Zenoh subscriber tooling.

## Executables

### sensors_read
- Collects readings from the Navio2 IMUs (MPU9250 and LSM9DS1), ADC, barometer, GPS and RCInput.
- Publishes each reading as a Zenoh sample using the topics listed below.
- Requires access to a Navio2 board and the corresponding sensors; the process exits early if the autopilot stack is detected.

Usage:

```bash
./sensors_read [--interval <seconds>] [--rc-channels <count>] [--once] [--log-level <LEVEL>] [--help]
```

Key options:
- `--interval` (`-t`): seconds between samples (minimum 0.1 s, default 1.0).
- `--rc-channels` (`-c`): number of RCInput channels to read (default 4, maximum 14).
- `--once` (`-o`): take a single snapshot then exit.
- `--log-level` (`-l`): set verbosity (`DEBUG`, `INFO`, `WARNING`, `ERROR`, `CRITICAL`; default `WARNING`).
- `--help` (`-h`): print the options summary.

### sensors_read_test
- Located in `test/subscriber.cpp` and built as the `sensors_read_test` executable.
- Opens a Zenoh session, subscribes to `telemetry/sensors/**`, and renders the latest values from every sensor in a simple terminal dashboard.
- Useful for verifying end-to-end publishing without additional tooling. The program runs until interrupted.

## Zenoh Topics

All samples share the `telemetry/sensors` base. Individual measurements are routed to:
- `telemetry/sensors/imu` – both IMU devices publish on this topic.
- `telemetry/sensors/adc` – ADC channel readings.
- `telemetry/sensors/barometer` – temperature and pressure.
- `telemetry/sensors/gps` – basic fix and position information.
- `telemetry/sensors/rcinput` – RC channel pulse widths.
- `telemetry/sensors/header` – reserved; currently unused.

## Payload Format

Every payload is plain-text `key=value` pairs separated by a single space and always includes a Unix `timestamp` (seconds since epoch). When a sensor cannot supply data, a short status string such as `GPS: unavailable` is emitted instead of key/value pairs.

### IMU (`telemetry/sensors/imu`)
- Example: `timestamp=1712072801 name=MPU9250 ax=0.11 ay=-0.02 az=9.79 gx=0.01 gy=0.00 gz=0.00 mx=0.12 my=-0.03 mz=0.45`
- Fields: `timestamp`, `name` (`MPU9250` or `LSM9DS1`), linear acceleration components `ax/ay/az`, gyroscope components `gx/gy/gz`, magnetometer components `mx/my/mz`.
- Units follow the Navio2 driver defaults (acceleration in g, angular rate in rad s⁻¹, magnetic field in gauss).

### ADC (`telemetry/sensors/adc`)
- Example: `timestamp=1712072801 a0=4.98 a1=4.96 a2=nan`
- Fields: `timestamp`, followed by one entry per available channel (`a0`, `a1`, …). Values are voltages derived from the raw millivolt readings (`raw / 1000`). Channels that fail to read are reported as `nan`.

### Barometer (`telemetry/sensors/barometer`)
- Example: `timestamp=1712072801 temperature=23.48 pressure=1012.67`
- Fields: `timestamp`, `temperature` (°C), `pressure` (mbar).

### GPS (`telemetry/sensors/gps`)
- Example: `timestamp=1712072801 fix_type=3 lat=52.2043 lon=0.1218 height=45.23`
- Fields: `timestamp`, `fix_type`, `lat`, `lon`, `height`.
- `fix_type` codes: `0` = no fix, `1` = dead reckoning, `2` = 2D, `3` = 3D, `4` = GNSS + dead reckoning, `5` = time-only.
- Position values are provided in degrees (latitude/longitude) and metres (height above ellipsoid) as returned by the Ublox driver.

### RC Input (`telemetry/sensors/rcinput`)
- Example: `timestamp=1712072801 roll=50 pitch=49 throttle=15 yaw=50`
- Fields: `timestamp`, `roll`, `pitch`, `throttle`, `yaw`.
- Values are stick deflections normalised to a 0-100 scale, where `0` maps to 1000 µs (minimum/disarmed), `50` represents the neutral 1500 µs position, and `100` corresponds to 2000 µs (full deflection).
- Channels that are unavailable when sampling are reported as `0` and logged as warnings.

## Notes

- The publisher reuses a Zenoh publisher per key expression; ensure the Zenoh daemon or peer is reachable before launching the binary.
- All payloads are emitted as UTF-8 strings. Downstream consumers can reuse the parsing logic from `test/subscriber.cpp` if needed.
