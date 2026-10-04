# Minimap generator

Draws a minimap (`Data/World<N>/mini_map.OZT`, 1024×1024, 32-bit TGA with the 4-byte OZT prefix)
from the walk map of a game map of the OpenMU server: a blue floor, rocky borders and pillars,
transparent outside. Only the largest connected walkable area is drawn.

The client positions the hero with the map X as the image row and the map Y as the column
(`NewUIMiniMap.cpp`), so the image is drawn that way.

Used for Kalima: all 7 Kalima maps (server maps 24–29 and 36) load `World25`.

```bash
docker exec database psql -U postgres -d openmu -t -A -c "SELECT encode(\"TerrainData\",'hex') FROM config.\"GameMapDefinition\" WHERE \"Number\"=24;" > kalima.hex
python make_kalima_minimap.py kalima.hex mini_map.OZT preview.png
tail -c 26 ../../src/bin/Data/World1/mini_map.OZT >> mini_map.OZT
```

The last line appends the TGA footer which the existing minimaps have. Copy `mini_map.OZT` to
`src/bin/Data/World25/` (tracked with `git add -f`, as `bin` is ignored) and to the `Data` folder of the build.
