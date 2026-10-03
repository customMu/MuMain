# Kalima instance, chamber of Kundun and Kundun Essence

Server features of this server (OpenMU plugins "Kalima instance", "Chamber of
Kundun" and "Kundun Essence currency"); the client shows them.

## Kalima and the chamber of Kundun

Kalima 1-7 are a daily instance, entered by talking to Lugard in Lorencia. They
are not in the warp window (M) anymore, and the Lost Map doesn't open a gate. The
weekly chamber of Kundun is entered by talking to David in Lorencia, with a Lost
Map of the level of the chamber.

## Kundun Essence

The essence is a currency: a counter of the character, earned in the Kalima
instance and the chamber of Kundun.

- **Balance:** shown in a line below the zen of the inventory, and at the bottom
  of the essence shop. `/essence` prints it into the chat.
- **Essence shop (Delgado):** the item tooltip shows the price in essence
  ("Not for sale" for items without a price) instead of the zen price.

The Symbols of Kundun are regular items again (5 symbols are combined to a Lost Map).

## Drop mode of the party

The party window has a button below the members, right of the close button, which
shows the drop mode of the party: free, random or in turn. A click of the party
master switches to the next mode; the button is grey for the other members. The
button is only shown when the server sent the mode.

## The real Kundun

The monsters 700-706 are Kundun 1-7 of the chamber: the model of the Illusion of
Kundun 7, bigger (scale 2.0 + 0.1 per level), with a red aura and the name
"Kundun". They use the attack animations and effects of the Illusion 7.

## Packets

The server and the client use custom packets with the head code `0xFB`:

| Packet | Direction | Content |
|---|---|---|
| `C1 08 FB 01` | server → client | essence balance, uint32 little endian |
| `C2 [size] FB 02 [count]` | server → client | `count` entries of store slot (byte) and price (uint32 little endian); count 0 = the opened shop sells for zen |
| `C1 05 FB 03 [mode]` | both | drop mode of the party: 0 free, 1 random, 2 in turn; the client sends it to change the mode |
