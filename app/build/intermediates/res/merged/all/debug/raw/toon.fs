#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D myTexture0;
uniform sampler2D myTexture1;
uniform sampler2D myTexture2;
uniform sampler2D myTexture3;
uniform sampler2D myTexture4;
uniform vec4 lumpos;
//uniform int u_alphatestenable;

varying vec2 vsoTexCoord;
varying vec2 vsoTexCoord2;
varying vec3 vsoNormal;
varying vec4 vsoModPosition;
varying vec4 vsoPosition;


void main(void) {

  float scale = vsoPosition.y;
    float pasX = vsoPosition.x;

    vec4 basecolor;

    vec3 lum = normalize(vsoModPosition.xyz - lumpos.xyz);
    //vec3 lum = normalize(lumpos.xyz - vsoModPosition.xyz);
    float intensity = dot(normalize(vsoNormal),-lum);
    //float intensity = dot(-lum, vsoNormal);
    float factor = 0.1;

    if( intensity > 0.1  && intensity <= 0.3) factor = 1.0;
    else if ( intensity > 0.3 && intensity <= 0.4) factor = 0.8;
    else if ( intensity > 0.4 && intensity <= 0.7) factor = 0.6;
    else if ( intensity > 0.7 && intensity <= 0.9) factor = 0.4;

    else factor = 0.2;

    if(scale > -10.0 && scale <= 5.0){
      basecolor = texture2D(myTexture0, vsoTexCoord);
      basecolor = vec4(factor, factor, factor, 0.5);
    }
    else if(scale > 5.0 && scale <= 10.0){
      basecolor = texture2D(myTexture1, vsoTexCoord);
      basecolor *= vec4(factor, factor, factor, 1.0);
    }

    else if(scale > 10.0 && scale <= 30.0){
      basecolor = texture2D(myTexture2, vsoTexCoord);
      basecolor *= vec4(factor, factor, factor, 1.0);
    }

    else if(scale > 30.0 && scale <= 50.0){
      basecolor = texture2D(myTexture3, vsoTexCoord);
      basecolor *= vec4(factor, factor, factor, 1.0);
    }

  	else{
      basecolor = texture2D(myTexture4, vsoTexCoord);
      basecolor *= vec4(factor, factor, factor, 1.0);
    }


/*    if(u_alphatestenable==1) {
        if(basecolor.a==0.0)
          discard;
     }*/
  	 gl_FragColor = basecolor;
}
