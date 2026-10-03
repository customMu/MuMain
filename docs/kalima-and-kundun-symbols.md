# Kalima instance and Symbols of Kundun

Server features of this server (OpenMU plugins "Kalima instance" and "Symbols of
Kundun currency"); the client only shows them.

## Kalima

Kalima 1-7 are a daily instance, entered by talking to Lugard in Lorencia. They
are not in the warp window (M) anymore, and the Lost Map doesn't open a gate.

## Symbols of Kundun

The symbols are a currency: a picked up symbol goes into a counter of the
character instead of the inventory.

- **Balance:** shown as a tooltip when the mouse is over the zen of the inventory,
  and at the bottom of the symbol shop. `/symbols` prints it into the chat.
- **Symbol shop (Delgado):** the item tooltip shows the price in symbols
  ("Not for sale" for items without a price) instead of the zen price.

The server sends two custom packets (head code `0xFB`):

| Packet | Content |
|---|---|
| `C1 08 FB 01` | balance, uint32 little endian |
| `C2 [size] FB 02 [count]` | `count` entries of store slot (byte) and price (uint32 little endian); count 0 = the opened shop sells for zen |
