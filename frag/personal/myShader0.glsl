#version 410

layout (location = 0) out vec4 FragColor;

in vec2 texcoord;

#pragma channel float cv0
float cv0_ = channel_cv0(texcoord);

#pragma channel float cv2
float cv2_ = channel_cv2(texcoord);

#pragma channel float cv3
float cv3_ = channel_cv3(texcoord);

#pragma channel float cv4
float cv4_ = channel_cv4(texcoord);

#pragma channel float cv5
float cv5_ = channel_cv5(texcoord);

#pragma channel float cv6
float cv6_ = channel_cv6(texcoord);

#pragma channel float cv7
float cv7_ = channel_cv7(texcoord);

#pragma channel float cv8
float cv8_ = channel_cv8(texcoord);

// #pragma channel mat3 mod_matrix

//palette colors
#pragma channel vec3 color0
vec3 color0_ = channel_color0(texcoord);
#pragma channel vec3 color1
vec3 color1_ = channel_color1(texcoord);
#pragma channel vec3 color2

#pragma channel int button0
int button0_ = channel_button0(texcoord);
#pragma channel int button1
int button1_ = channel_button1(texcoord);
#pragma channel int button2
int button2_ = channel_button2(texcoord);

#pragma channel vec3 texFB

uniform float iTime;

#define PI 3.14159265358979323846
#define TWO_PI 6.28318530718

#define repeat(v) mod(p + 1., 2.) -1.
#define un(a, b) min(a, b)

float random (float x) {
	return fract(sin(x) * 1e4);
}

float random (in vec2 st) {
	return fract(sin(dot(st.xy,
	                     vec2(12.9898, 78.233))) *
	             43758.5453123);
}

float noise (in vec2 st) {
	vec2 i = floor(st);
	vec2 f = fract(st);

	// Four corners in 2D of a tile
	float a = random(i);
	float b = random(i + vec2(1.0, 0.0));
	float c = random(i + vec2(0.0, 1.0));
	float d = random(i + vec2(1.0, 1.0));

	vec2 u = f * f * (3.0 - 2.0 * f);

	return mix(a, b, u.x) +
	       (c - a) * u.y * (1.0 - u.x) +
	       (d - b) * u.x * u.y;
}

// Color Space Conversion \-----------------------------|
vec3 rgb2hsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


//--------ROTATION-----------------------------------------|

mat2 getRotationMatrix(float _angle){
	return mat2(cos(_angle), -sin(_angle),
	            sin(_angle), cos(_angle));
}

vec2 rotate2D(vec2 _st, float _angle) {
	_st -= 0.5;
	_st =  mat2(cos(_angle), -sin(_angle),
	            sin(_angle), cos(_angle)) * _st;
	_st += 0.5;
	return _st;
}

vec2 rotate2D(vec2 _st, mat2 rotation_matrix ){
	_st -= 0.5;
	_st *= rotation_matrix;
	_st += 0.5;
	return _st;
}


//-----------------------------------------|

vec3 box(vec2 _st, vec2 _size, float _smoothEdges) {
	_size = vec2(0.5) - _size * 0.5;
	vec2 aa = vec2(_smoothEdges * 0.5);
	vec2 uv = smoothstep(_size, _size + aa, _st);
	uv *= smoothstep(_size, _size + aa, vec2(1.0) - _st);
	return vec3(uv.x * uv.y);
}

float circle(vec2 _st, float _radius) {
	vec2 l = _st - vec2(0.5);
	return 1. - smoothstep(_radius - (_radius * 0.01),
	                       _radius + (_radius * 0.01),
	                       dot(l, l) * 4.0);
}

float circle(vec2 _st, float _radius, float thickness) {
	return circle(_st, _radius) - circle(_st, _radius - thickness);
}

vec3 horizontalLine(vec2 st, float pos, float size, vec3 color) {
	st.x = fract(st.x) - pos ;
	return (step(0., st.x  ) * 1.0 - step( size,  st.x  )) * color;
}

vec3 verticalLine(vec2 st, float pos, float size, vec3 color) {
	st.y = fract(st.y) - pos ;
	return (step(0., st.y  ) * 1.0 - step( size,  st.y  )) * color;
}



vec2 toPolar(vec2 st) { // Outputs distance 0 to 1 and Thet
	st = st * 2. - 1.;

	// Angle and radius from the current pixel
	float ang = ( atan(st.y, st.x) + PI ) / TWO_PI; // angle is scaled so you can use this to address stuff and be more shader like
	float rad = sqrt(dot(st, st)) ;

	st = vec2(ang, rad );

	return st;
}

mat3 rotateX(float a) {
	return mat3(
	           1.0, 0.0, 0.0,
	           0.0, cos(a), sin(a),
	           0.0, -sin(a), cos(a)
	       );
}

mat3 rotateY(float a) {
	return mat3(
	           cos(a), 0.0, sin(a),
	           0.0, 1.0, 0.0,
	           -sin(a), 0.0, cos(a)
	       );
}

#define TRIAD vec2(0.33333333, 0.66666666)

//Color Quantizer ( Color Tables? )
vec3 pallete(float root) {
	return vec3( root, root + TRIAD[0], root +  TRIAD[1] ); //return a quantized color scheme
}

vec3 colorizer( float amplitude,  float color, float saturation ) {
	float value = (amplitude - 0.5) * 2.;
	return hsv2rgb( vec3(color, saturation, value) );
}

float impulse( float k, float x ) {
	float h = k * x;
	return h * exp(1.0 - h);
}

float sinc( float x, float k ) {
	float a = PI * ((float(k) * x - 1.0) ); //const
	return sin(a) / a;
}

float polygon(vec2 st, float frequency, float phase, float spacing, float r) {
	r = TWO_PI/r;
	float d = 0.0;
	// Remap the space to -1. to 1.
	st = st * 2. - 1. ;
	st.x += phase;

	st = fract(st * frequency)*spacing;

	// Angle and radius from the current pixel
	float a = atan(st.x, st.y) + PI ;


	// Shaping function that modulate the distance
	d = cos(floor(.5 + a / r) * r - a) * length(st) ;

	return 1.0 - step(0.5, d);

}

#define TRIR 2.09439510239

float osc(int shape, vec2 scan, float frequency, float phase, float drift) {
	float ret = 0.0;
	//strange syntax bc contidionals are dumb in shaders
	ret += (sin(PI * scan.x * frequency + (drift * iTime) + phase )  ) * float(shape == 0);
	ret += (tan(PI * scan.x * frequency + (drift * iTime)) + phase) * float(shape == 1);
	ret += polygon(scan, frequency, (drift * iTime/ 8.), 1. ,  4.) * float(shape == 2);
	ret += (fbm( (scan * (2.* frequency)) + vec2(drift * iTime / 2.) ,  0.2 , 1. , 0.2 ) * 2. - 1.) * float(shape == 3);

	return ret;
}

// not in use
// float getScan(vec2 position, float polar) {
// 	vec2 polarPos = toPolar(position) ;
// 	// vec2 swappedPolar = vec2( polarPos.y  , polarPos.x );
// 	polarPos = vec2( sin((polarPos.x ) * PI ) , polarPos.y  ) ;

// 	float ret = mix( ((position.x * 100.) + (position.y * 100.) * 100.) / 1000., sin(polarPos.y  * 8.) , polar);
// 	return ret;
// }

vec2 getScan2D(vec2 position, float polar, bool polar_flip) {

	vec2 polarPos = toPolar(position) ;
	polarPos = vec2( sin( (polarPos.x ) * PI)  , polarPos.y   );
	vec2 wrappedPolar = vec2( polarPos.x , sin(polarPos.y * PI ));

	polarPos = vec2(polarPos.y  );

	// vec2 polarPos = toPolar(position) ;
	// vec2 swappedPolar = vec2( polarPos.y  , polarPos.x );
	// polarPos = vec2( sin(polarPos.x * PI ) , polarPos.y  ) ;

	// float ret = mix( ((position.x * 100.) + (position.y * 100.) * 100.)/1000., sin(polarPos.y * 8.) , polar);

	vec2 ret = mix(mix( position, wrappedPolar , polar), mix( position, polarPos , polar), float(polar_flip));
	return ret;
}


// divides cv into 2 values 1: 0.5 to 1 -> 0 to 1
// 							2: .5 to 0 -> o to 1
vec2 splitCV(float cv) {
	vec2 ret = vec2(0.0, 0.0);
	if ( cv >= 0.5) {
		ret[0] = (cv - 0.5) * 2.;
	}

	if (cv < 0.5) {
		ret[1] = (abs(cv - 1.) - 0.5) * 2.;
	}

	return ret;
}

vec3 mixMode(vec3 shape, vec3 osc, float cv, int mode) {
	vec2 cv2_split = splitCV(cv);
	//osc 1 0: min/max 1: +/- 2: * //

	if ( mode == 0) {
		shape =  ( shape + ( osc  * cv2_split[0]) + shape - (osc * cv2_split[1]) ) ;
	}
	else if ( mode == 1) {
		shape = ( pow(shape,  ( osc  * cv2_split[0])) + shape / (osc * cv2_split[1]) ) ;
	}
	else {
		shape = ( max(shape, ( osc  * cv2_split[0])) + min( shape, (osc * cv2_split[1]) ) ) ;
	}
	return shape;
}

float mixMode(float shape, float osc, float cv, int mode) {
	vec2 cv2_split = splitCV(cv);
	//osc 1 0: min/max 1: +/- 2: * //

	if ( mode == 0) {
		shape =  ( shape + ( osc  * cv2_split[0]) + shape - (osc * cv2_split[1]) ) ;
	}
	else if ( mode == 1) {
		shape = ( shape * ( osc  * cv2_split[0]) + shape / (osc * cv2_split[1]) ) ;
	}
	else {
		shape = ( max(shape, ( osc  * cv2_split[0])) + min( shape, (osc * cv2_split[1]) ) ) ;
	}
	return shape;
}


void datBoiTest() {
	float scale = 1.;
	vec2 position = texcoord * scale;

	vec2 rot;
	rot[0] = (cv4_) * TWO_PI;
	rot[1] = (cv5_) * TWO_PI;
	mat2 rotation_mat0 = getRotationMatrix(rot[0]);
	mat2 rotation_mat1 = getRotationMatrix(rot[1]);

	float feedback_scalar = (((clamp(cv3_ , 0.75, 1.) /0.75) - 1.) * 2. + ((clamp(1.-cv3_ , 0.75, 1.) /0.75) -1.) * 2.) *2.;


	//SIN OSC

	//vec2 scan0 = vec2(getScan(position, rot[0], cv6_));

	vec2 scan0 = getScan2D( rotate2D(position, rotation_mat0) , cv6_, false);

	float syncDrift = 2. * cv8_ + .01;

	float f_scale = 15.;
	float f1 = cv0_ * f_scale;

	float f2 = cv2_  * f_scale;

    // DANIMAL commented this out
	// mod_matrix[0][0];

	float osc1, osc2;

	vec2 scn1pos = vec2( position.x, position.y );
	//cross modulate here?!
	vec2 scan1 = getScan2D(rotate2D(position, rotation_mat1) , cv7_, true );

	osc1 = osc(button0_, scan0, f1, 0., syncDrift  ) ;
	//cross modulation!!!
	//scan1 += (osc1 * ((cv0_  * 1.5) + 0.5) ) * float(button2_);
	osc2 = osc(button2_, scan1, f2, 0. , syncDrift);

	//vec3 waveA = drawImage(texcoord + vec2(1., iTime/10. * syncDrift), scan1 , vec2(f2, 1. )  );

	float fb_darken = max(0.5 * (2. * (1. - (feedback_scalar * .91) )), float(button1_ == 0 || button1_ == 2));
	fb_darken += float(button1_ == 3) * 0.3   ;
	vec3 oscA = (color0_ * (osc1 - .5) * 2. ) * fb_darken  ; //colorizer(osc1, hue[0],  cv0_ - 1.) ;
	vec3 oscB = (color1_ * (osc2 - .5) * 2. ) * fb_darken  ; //colorizer(osc2, hue[1],  cv2_  ) ;

	//Mixer
	vec3 color = vec3(0.);
	//turn osc off at low cv vals
	color += step(0.017, cv0_) * oscA ;
	color += step(0.017, cv2_) * oscB ;


	//Feedback
	//vec2 polar_fb_position = toPolar(position);
	float zoom = 0.1 * feedback_scalar;
	vec2 fbPos = position;
	vec2 fbPos2 = position;
	
	if(button1_ == 3){// rotating feedback
		//fbPos += zoom;
		fbPos = rotate2D(fbPos, rotation_mat0);//-PI * feedback_scalar/5.4);
		fbPos2 = rotate2D(fbPos, rotation_mat1);
	}
	else{
		fbPos2 = fbPos;
	}
	if ( button1_ > 1) {
		fbPos -= zoom / 2.2;
		fbPos = clamp(fbPos * (1. + zoom) , vec2(0.), vec2(1.)) ;
	}


	float in_frame = (1. - step(1., fbPos.x)) * (1. - step(1., fbPos.y))  * float(fbPos.x > 0.) * float(fbPos.y > 0.) ;
	float in_frame_circle = (1.-step(.5, distance(vec2(.5), fbPos)) );
	vec3 fbColor = channel_texFB(fbPos).xyz * 0.5 + channel_texFB(fbPos2).xyz * 0.5 ;

	//Zooming Feedback (Analog Style)
	fbColor = mix(fbColor * in_frame * (feedback_scalar*2.), fbColor, float( button1_ == 2));
	if(button1_ == 2){
		color += fbColor * feedback_scalar * 3., vec3(0.0);
	}

	//Basic Trailing Feedback Mode
	if ( button1_ == 0) {
		color += mix( fbColor * feedback_scalar * 3., vec3(0.0) , float(!(color.x  < 0.1 || color.y  < 0.1 || color.z  < 0.1)) );
	}

	//Hyper-Digital Feedback Mode
	fbPos += float( button1_ == 1) * sin(vec2(2. * feedback_scalar * cv4_, 2. * feedback_scalar * cv5_) * TWO_PI) ;
	if (button1_ == 1 ) {
		color = (color / (fbColor - .01) * 20. * feedback_scalar * float(color.x  > 0.1 || color.y  > 0.1 || color.z  > 0.1) ) + fbColor;
	}
	
	//Rotating Feedback
	if (button1_ == 3 ) {
		color += fbColor / feedback_scalar * .9;
	}

	color *= clamp((cv3_ * 2.) - 1., -0.75, 0.75) /.75 ; //master dimmer
	FragColor = vec4( color, 1.0 );

}

void main( void ) {
	datBoiTest();
	//gl_FragColor = vec4(drawImage(texcoord , vec2(0.,0.), vec2(1.)), 1.0);
	// gl_FragColor = vec4(vec3(cv0_), 1.0);
}
