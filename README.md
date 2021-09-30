# Apollo
Attempt to integrate Razer Chroma lighting effects for [SuperGiantGames Hades](https://store.steampowered.com/app/1145360/Hades/).

Once saw a reddit post showing how their controllers light up according to the god boons their getting. 

Feelsbadman no such functionalities for PC/MKB players even where there's so much potential for RGB goodness.

# Methodology

As of now, here's what I've managed to achieve so far.

- Hades `EngineWin64s.dll` is patched with [Ghidra](https://ghidra-sre.org/), so that my `ApolloInjector.exe` can hook a specific function call
- Specifically `SDL2::SDL_JoystickSetLED` from `SDL2.dll`, an [API](https://wiki.libsdl.org/SDL_JoystickSetLED) which responsible for setting the lights for controller
- Scrapped some functions of from the official Razer Chroma SDK [examples](https://assets.razerzone.com/dev_portal/C%2B%2B/html/en/_chroma_s_d_k_impl_8cpp-example.html)
- Hooked the function call and replace it with some razer chroma effects whenever the loots/boons are from gods then return the appropriate function back

# Limitation

- Currently, the patched `EngineWin64s.dll` will still require any controller to be connected so the `SDL2.dll` function call actually being triggered
- Reckon would require a little bit more patching on `EngineWin64s.dll` so that the call will be triggered without any controller connected
 
 # Credits
 
 Huge thanks to [nbusseneau](https://github.com/nbusseneau/) the author of [hephaistos](https://github.com/nbusseneau/hephaistos), which offers the best ultrawide experience you could ever get from Hades, for 
 taking his time helping me throughout the overall process of reverse engineering, memory injection & patching, plus some x86 assembly :D
 
 Also, of course [SupergiantGames](https://www.supergiantgames.com/) themselve for this masterpiece.
