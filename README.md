# DemoRecordFix_mm

**Metamod plugin for fixing SourceTV (HLTV) disconnects on CS2 servers.**

---

## What it does

* Blocks disconnect of HLTV (`CServerSideClient`) when `tv_enable` is enabled
* Expands `maxPlayers` by +1 on server startup when `tv_enable` is enabled
* Fixes HLTV breaking when the match restarts on the same map (no `changelevel`)

---

## Result

* HLTV is not kicked
* Demo recording works correctly

---

## Requirements

* Metamod:Source (CS2)
* `tv_enable 1`

---

## Author

Slynx (˙·٠● S l y n x ●٠·˙)
[https://slynxdev.cz](https://slynxdev.cz)