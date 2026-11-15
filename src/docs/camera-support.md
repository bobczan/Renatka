# Speed camera support

Reviewed 20 October 2026. This table controls GPS database alerts, including camera CSV imports. It does not describe radar receivers or jammers. The interface language never changes the location policy.

**Enabled (19 countries):** Austria (AT), Belgium (BE), Czechia (CZ), Kingdom of Denmark (DK), Spain (ES), Finland (FI), United Kingdom (GB), Croatia (HR), Hungary (HU), Italy (IT), Luxembourg (LU), Kingdom of the Netherlands (NL), Norway (NO), Poland (PL), Portugal (PT), Serbia (RS), Russia (RU), Sweden (SE), Slovenia (SI).

**Exact alerts blocked:** Switzerland (CH), Republic of Cyprus (CY), Germany (DE), France (FR). No invented French danger-zone mode is substituted.

**Other territories:** disabled pending a GPS-specific policy review.

## In-car settings

| Setting | Values | Effect |
|---|---|---|
| Camera alerts | Off / Visual | Off hides markers and the approach card; Visual shows them. |
| Camera notifications both directions | On / Off | On ignores the +/-45 degree heading filter, so a camera facing the opposite carriageway still alerts. Off warns only for the direction the record enforces. |
| Temperature | On / Off | Shows the outside temperature beside the clock. The value comes from the `vstatd` helper through `/tmp/cltemp`, falling back to `temp=` in `/tmp/cfstate`. |

## Sound cue (parked)

The audible camera cue is not released. The tone is generated and `/dev/barc-snd` accepts the stream
(`BARC-SND OPEN`, `CUE START`), but the radio answers the navigation audio interruption with
**"INTERRUPTION NA"** and never switches its input to the navigation audio line, so nothing reaches the
speakers. The factory navigator requested that interruption over the Focus bus; reproducing that handshake
is the remaining work. With Bluetooth media playing the cue is mixed into the stream correctly, but the
car still mutes the navigation line.

The implementation is kept dormant: `clioa2dp` still reads `/tmp/clalert` and synthesises the two-tone cue,
and `audio_chime.h` holds the waveform. Nothing writes the request any more, and the settings row was removed.
