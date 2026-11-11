# Blockline — Roblox urban shooter

An original urban team arena shooter starter with a generated city, two teams, a rifle, aiming, reloading, sprinting, practice targets, and five-minute matches.

## Play in Roblox Studio

1. Download [Blockline.rbxlx](./Blockline.rbxlx) using the **Download raw file** button on its GitHub page.
2. In Roblox Studio, choose **File → Open from File** and open the downloaded file.
3. Press **Play**. The map generates at runtime; wait for the ten-second countdown.
4. For multiplayer, use **Test → Server & Clients** with two clients.

See [START-HERE.txt](./START-HERE.txt) for controls, customization, and rules. Editable scripts are in [Server.luau](./Server.luau) and [Client.luau](./Client.luau); copies are embedded in the place file.

## Controls

WASD to move, mouse to look, click to shoot, right click to aim, R to reload, Shift to sprint, Space to jump. Controller and touch action bindings are included.

## Validation

Place XML and embedded script round-trip checks passed. Both scripts passed a Lua 5.4 syntax check using a compatible subset. Roblox Studio playtesting is still required; it was not available in the build environment.

This prototype uses original generated parts and no Urban Reload assets or code. Persistence, custom weapon animation, audio, and monetization are not included.
