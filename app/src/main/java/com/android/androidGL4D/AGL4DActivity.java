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

import android.app.Activity;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import com.google.vrtoolkit.cardboard.CardboardActivity;
import com.google.vrtoolkit.cardboard.CardboardView;


public class AGL4DActivity extends CardboardActivity {

    private AGL4DView cardboardView;
    private Button up, down, right, left, vrButton;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.common_ui);

        cardboardView = (AGL4DView) findViewById(R.id.cardboard_view);

        setCardboardView(cardboardView);

        CardboardOverlayView overlayView = (CardboardOverlayView) findViewById(R.id.overlay);

        up = new Button(this);
        down = new Button(this);
        right = new Button(this);
        left = new Button(this);
        vrButton = new Button(this);

        up.setText("^");
        down.setText("v");
        right.setText(">");
        left.setText("<");

        vrButton.setText("Toggle Vr");
        //vrButton.setVisibility(View.GONE);

        up.setVisibility(View.GONE);
        down.setVisibility(View.GONE);
        right.setVisibility(View.GONE);
        left.setVisibility(View.GONE);

        up.getBackground().setAlpha(128);
        down.getBackground().setAlpha(128);
        right.getBackground().setAlpha(128);
        left.getBackground().setAlpha(128);
        vrButton.getBackground().setAlpha(128);

        vrButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                boolean vrmode = cardboardView.getVRMode();
                if(!vrmode) { //pas en mode vr, on active les buttons
                   up.setVisibility(View.GONE);
                    down.setVisibility(View.GONE);
                    right.setVisibility(View.GONE);
                    left.setVisibility(View.GONE);
                }
                else {
                    up.setVisibility(View.VISIBLE);
                    down.setVisibility(View.VISIBLE);
                    right.setVisibility(View.VISIBLE);
                    left.setVisibility(View.VISIBLE);
                }
                cardboardView.setVRModeEnabled(!vrmode);
            }
        });


        LinearLayout crossButtons1 = new LinearLayout(this);
        LinearLayout crossButtons2 = new LinearLayout(this);

        RelativeLayout.LayoutParams crossButtonsL = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT
                , RelativeLayout.LayoutParams.WRAP_CONTENT);

        RelativeLayout.LayoutParams crossButtonsR = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT
                , RelativeLayout.LayoutParams.WRAP_CONTENT);


        crossButtons1.setOrientation(LinearLayout.VERTICAL); crossButtons2.setOrientation(LinearLayout.HORIZONTAL);
        crossButtons1.setGravity(Gravity.BOTTOM | Gravity.RIGHT); crossButtons2.setGravity(Gravity.BOTTOM | Gravity.LEFT);

        up.setLayoutParams(crossButtonsR);
        down.setLayoutParams(crossButtonsR);
        left.setLayoutParams(crossButtonsL);
        right.setLayoutParams(crossButtonsL);

        crossButtons1.addView(up);
        crossButtons1.addView(down);
        crossButtons2.addView(left);
        crossButtons2.addView(right);


        this.addContentView(crossButtons1, new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.FILL_PARENT
                , RelativeLayout.LayoutParams.FILL_PARENT));
        this.addContentView(crossButtons2, new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.FILL_PARENT
                , RelativeLayout.LayoutParams.FILL_PARENT));
        this.addContentView(vrButton, new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT
                , RelativeLayout.LayoutParams.WRAP_CONTENT));


    }

    @Override
    public void onCardboardTrigger() {

        if(cardboardView.getVRMode()) {// si on est en vr
            if(vrButton.getVisibility()==View.GONE)
                vrButton.setVisibility(View.VISIBLE);
            else vrButton.setVisibility(View.GONE);
        }

        super.onCardboardTrigger();
    }

    public boolean isUpPressed() {
        return up.isPressed();
    }

    public boolean isDownPressed() {
        return down.isPressed();
    }

    public boolean isRightPressed() {
        return right.isPressed();
    }

    public boolean isLeftPressed() {
        return left.isPressed();
    }

}
