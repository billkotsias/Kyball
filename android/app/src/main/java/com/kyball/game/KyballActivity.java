package com.kyball.game;

import org.libsdl.app.SDLActivity;

/** SDL2 activity. Touches are read as SDL finger events by HIDBoss. */
public class KyballActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "kyball" };
    }

    @Override
    protected String[] getArguments() {
        return new String[] { "--touch" };
    }
}
