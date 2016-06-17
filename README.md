# agarIO
This is a simplified version of [AgarIO](http://agar.io/) game, developed for networking subject.

It contains a [client](game/U-gine/src/client.cpp), which is inside *U-gine* project because it needs to render entities. On the other side, the [server](game/Server/Server.cpp) uses no graphic interface.

This project uses *enet2.0* (with a Wrapper given by networking teacher) and *pthreads*.
