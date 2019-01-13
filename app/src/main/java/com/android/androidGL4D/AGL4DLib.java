/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.androidGL4D;

// Wrapper for native library

import android.content.res.AssetManager;

public class AGL4DLib {

     static {
         System.loadLibrary("androidGL4D");
     }

    public static native void init(AssetManager assetManager, String vshader, String fshader,
                                   String toonshader, String fnightbasicshader, String fnightbasictoonshader);
    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void reshape(int width, int height);
     public static native void draw(float[] eyeView, float[] eyePerspective);
    public static native void setcamera(float[] headviewv,float[] forwardv, float[] upv, float[] rightv, float forward1v, float forward2v);
    public static native void event(int x_left, int z_up, int x_right, int z_down);
    public static native void quit();
}
