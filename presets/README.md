# Categorized preset catalog

8 original Factory programs + 72 supplied category entries = 80 native programs.

| Category | Entries |
| --- | ---: |
| Master | 13 |
| Vintage | 10 |
| Lo-Fi | 5 |
| Clean | 4 |
| Pushed | 11 |
| Bright | 8 |
| Vocal | 5 |
| Bass | 3 |
| Classic | 7 |
| Drums | 4 |
| Guitars | 2 |

`imported-bank.json` preserves the supplied values. `catalog.json` records unique IDs, mapped parameters and notes for ambiguous fields. Exported bank files use category directories to avoid collisions. Single explicit EQ choices are preserved. Ambiguous EQ at 15 IPS uses CCIR for Master/Clean/Bright/Vocal and NAB for other categories; low-speed ambiguous EQ uses NAB. Sync/Repro selects REPRO. These are engineering starting choices, not recovered proprietary settings. `engineering-decisions.json` lists the ten EQ decisions changed in 0.1.3. See the main README for the complete policy and limitations.
