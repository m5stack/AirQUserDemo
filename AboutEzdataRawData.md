# EZDATA `raw` Data Format Change (v0.1.2 → v0.2.0)

## Summary

| | v0.1.2 | v0.2.0 |
|---|--------|--------|
| `dataType` | `string` | `dict` |
| `value` | JSON string | JSON object |
| Read | `JSON.parse(value)` required | use `value` directly |

Inner sensor fields (`sen55` / `scd40` / `rtc` / `profile`) are **unchanged**.

## v0.1.2

```json
{
  "dataType": "string",
  "name": "raw",
  "value": "{\\\"sen55\\\":{\\\"pm2.5\\\":12.6,...},\\\"scd40\\\":{...},\\\"rtc\\\":{...},\\\"profile\\\":{...}}"
}
```

## v0.2.0

```json
{
  "dataType": "dict",
  "name": "raw",
  "value": {
    "sen55": {"pm1.0": 3.5, "pm2.5": 3.7, "humidity": 36.55, "temperature": 36.03, "voc": 0},
    "scd40": {"co2": 751, "humidity": 34.54, "temperature": 37.18},
    "rtc": {"sleep_interval": 60},
    "profile": {"nickname": "AirQAirQ"}
  }
}
```

## Consumer Compatibility

```js
const raw = (data.dataType === "dict" || typeof data.value === "object")
  ? data.value
  : JSON.parse(data.value);
```
