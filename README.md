# Blackiron Monkey

Was originally intended for me to use during gamejams, and has a lot of amenities to make 
rapid development very easy.

It was easy to build in a unity fashion, and was able to have the same code work for a static (release) and
dynamic dll (debug) build, which allowed you to hotreload a vast majority of assets (sound/images/whatever) with
the engine. It also featured a simple lisp parser as a primary data format and a develop console with autocomplete.

There is also a infrequently tested but last confirmed working web deployment path using emscripten.

It was basically a small framework. I found that it was simpler to just stick to making my own mini frameworks instead
of a generic one.

Although I did comeback full circle and realize that investing in tooling code would be invaluable. Of course the current project
I'm working on is too in progress for me to do this (or rather, I believe the time I would invest in making such a change would
be impractical.)
