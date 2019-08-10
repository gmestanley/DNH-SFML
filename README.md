# Touhou Danmakufu ph3.5 ~ Woo Edition
All hail our lord and savior Mima and our queen, Kogasa's Woo. This version of Danmakufu is made for the purposes of both optimization and fixing some issues with the original source that mkm dropped. (See James7132's repo for a link to the original download) <b>The master branch version of this repo is completely backwards compatible with ph3. This branch will contain functions that are not in original PH3.  If something doesn't work as expected, please let me know.</b> My Discord tag is WishMakers#0426 if you need to reach me.

## Changes To New Feature Branch
The original creator of each change will be listed here in parentheses next to the new function/change's listing.
 * Added "solid_pixel" tag to shot data definitions (gtbot)
	- In a shot data definition, if you add solid_pixel = true, the bullet will be rendered on solid pixels, preserving image quality.
 * Added SetTextureRenderMethod (gtbot)
	- Argument 1: Texture Width/Height determination
	- Determines how future loaded textures' internal widths and heights are determined
		* D3DX_DEFAULT: Textures will be rounded up to a power of 2 (blurry if not in a power of 2, but faster)
		* D3DX_DEFAULT_NONPOW2: Textures will not be rounded up to a power of 2 (crisper image if not in a power of 2, but slower)
	- Keep in mind that changes to this will only affect textures loaded after this function was called.
	- D3DX_DEFAULT is the default value.
 * Added SetShotDelayRenderBlendType (gtbot)
    - argument 1: Blend Type
    - Globally forces all delay clouds to use the specified blend type
    - use BLEND_NONE to disable this. BLEND_NONE is the default.
 * Added GetShotDelayRenderBlendType (gtbot)
    - returns the value that's specified in SetShotDelayRenderBlendType    
 * Added ObjSpell_SetEraseShotType (gtbot)
    - argument 1: ObjSpell ID
    - argument 2: bullet collision behaviour 
    - Sets the behaviour of ObjSpells when colliding with an enemy bullet
    - TO_TYPE_ITEM is the default value
      * TO_TYPE_ITEM: Deletes bullets and spawns an item 
      * TO_TYPE_IMMEDIATE: Immediately deletes bullets
      * TO_TYPE_FADE: Sets bullets to fade out and delete
	  * TO_TYPE_MOVEMENT_FREEZE: Stops the movement processing of bullets
	  * TO_TYPE_MOVEMENT_RESTORE: Restores the movement processing of bullets
	- Exercise caution when using TO_TYPE_MOVEMENT_FREEZE, as the bullets will never move again until you restore their movement.
	- ObjMove_SetProcessMovement can also restore these frozen bullets, and vice versa
 * Added ObjShot_SetEraseShotType (gtbot)
    - Argument 1: ObjShot ID
    - argument 2: bullet collision behaviour 
	- When bullets are set to erase shots, their behaviour can be changed with this function.
	- Argument 2 accepts the same constants found in ObjSpell_SetEraseShotType.
    - TO_TYPE_ITEM is the default value
 * Added ObjMove_SetProcessMovement (gtbot)
	- Argument 1: object ID
	- Argument 2: set movement processing on/off (boolean)
	- Determines whether the specified object will run their movement process or not.
	- Exercise caution when using ObjMove_SetProcessMovement, as objects will never move again until you restore their movement.
	- true is the default value.
 * Added ObjMove_GetProcessMovement (gtbot)
	- Argument 1: object ID
	- returns the value that's specified in ObjMove_SetProcessMovement

 
 ## Changes To Master Branch
Here's a list of changes compared to original Danmakufu that may enable your computer to use it better.
 * Changes the way RNG is determined to be much more stable than previously (mt19937 compared to a Mersenne Twister from 1996, lmao)
 * Removes a lot of repeated function calls (in bullet movement only as of 6/24/2019)
 * Removes a LOT of unnecessary vertices calculated (but not used) when drawing curvy lasers (see Special Thanks)
 * Enables a lot of optimization flags that mkm for some reason disabled (basically everything under /O2)
 * Attempts to fix the window resolution issue (not quite 640x480) that occurs with the source when built, but not the original ph3 build.
 * Dat extraction now works properly, enables the use of games like Kaisendo's Hollow Song of Birds and other games that use dat archives.
 * Side bars in fullscreen are now black as opposed to the ubiquitous DNH Gray.

## Requirements
 * zlib
</br>Best and recommended way to obtain it is to use [vcpkg](https://github.com/Microsoft/vcpkg) C++ Library Manager.

## Known Issues
 * GetEventArgument supposedly doesn't work with object IDs now?  Unconfirmed.
 * Wine 4.12.1 (confirmed on macOS at least) suffers some scaling problems with the window size, being 9 pixels too wide and 7 pixels too tall.  This causes some nasty scaling on in-game assets, possibly a result of old Windows size calls not being 100% compatible with Wine releases.
 * The exe will crash when trying to load a sound file that is both: not 1411kbs and is stereo
 
## Special Thanks
Natashi - for the code that made curvy lasers run a lot better than what I could do
</br>Trickysticks - for the woo art that graced the world
</br>UltimaOmega - for helping me with my sorry excuse for zlib code

## Contributions
Unlike the original source, this version is *technically* in active development (though my primary focus will be Danmakufu Remastered - check out Ultima's repo for that here) so pull requests are absolutely accepted provided they don't break the game. Though bear in mind that as this version of DNH still relies on mkm's original source code (most of which is either very unoptimized even with optimization flags or is very outdated in execution), there is only so far we can go in attempting to make it better.
</br>In addition, *this build will be focused on optimization and performance improvements only.* If you want to suggest brand new features, bug fixes, etc. please turn your attention to Danmakufu Remastered instead - this version is made to be completely backwards compatible, and as such new features can't simply be added.

## License
zlib library has its own license, please check zlib.h in the repo for that information.</br></br>
(quoted from James7132's repo of the original source, maintaining the same license.) </br>This code is licensed under the NYSL (Niru nari Yaku nari Suki ni shiro License'). Main translated points:

 * "No warranty" disclaimer is explicitly included.
 * Modified version of the software MUST be distributed under the responsibility of the distributer.
 * Official version is written in Japanese.
