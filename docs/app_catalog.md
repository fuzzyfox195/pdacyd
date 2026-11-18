# Start Screen application catalog

Source of truth: `src/ui/screens/start_screen/start_screen_apps.c`. The catalog order, IDs, user-facing names, icons, colors, tile sizes, positions, and preview strings are retained. Inspection found no tile click callbacks or functional app implementations in the original source tree. Therefore all eight entries are classified as launchable apps and have temporary `Coming soon` screens. No entry was inferred to be a system action or widget because the source models every one as an app tile and provides no alternate action behavior.

| Order | Preserved ID | Display name | Classification | Current implementation |
|---:|---|---|---|---|
| 1 | `weather` | Weather | Launchable application | Weather-owned placeholder; original tile preview retained |
| 2 | `voice_note` | Voice note | Launchable application | Voice note-owned placeholder |
| 3 | `music` | Komposer | Launchable application | Komposer-owned placeholder; original tile preview retained |
| 4 | `setting` | Setting | Launchable application | Setting-owned placeholder |
| 5 | `find_phone` | Find phone | Launchable application | Find phone-owned placeholder |
| 6 | `calendar` | Calendar | Launchable application | Calendar-owned placeholder |
| 7 | `timer` | Timer | Launchable application | Timer-owned placeholder |
| 8 | `stopwatch` | Stopwatch | Launchable application | Stopwatch-owned placeholder |

Each Start Screen tile passes its existing ID to the centralized registry/manager. The registry contains each launchable app once and keeps the same order. The catalog is not a second independently rendered UI list.
