#version 100

uniform sampler2D myTexture[5];
uniform vec4 lumpos;
varying vec2 vsoTexCoord;
varying vec3 vsoNormal;
varying vec4 vsoModPosition;
varying vec4 vsoPosition;

//mediump vec4 gl_FragColor;
precision mediump float;

void main(void) {
  
  float scale = vsoPosition.y;

  if(scale > -10.0 && scale <= 5.0){
  gl_FragColor = texture2D(myTexture[0], vsoTexCoord);
  }
  else if(scale > 5.0 && scale <= 10.0){
  gl_FragColor = texture2D(myTexture[1], vsoTexCoord);
  }

  else if(scale > 10.0 && scale <= 30.0){
  gl_FragColor = texture2D(myTexture[2], vsoTexCoord);
  }

  else if(scale > 30.0 && scale <= 50.0){
  gl_FragColor = texture2D(myTexture[3], vsoTexCoord);
  }
  else{
  gl_FragColor = texture2D(myTexture[4], vsoTexCoord);
  }
}
