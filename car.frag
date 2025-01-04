 #version 330 compatibility
 in  vec2  vST;
 in  vec3  vN;			// normal vector
 in  vec3  vL;			// vector from point to light
 in  vec3  vE;			// vector from eye to car
 
 uniform sampler2D  uTexUnit;
 uniform float   uKa, uKd, uKs;		// coefficients of each type of lighting
 uniform float   uShininess;		// specular exponent

 const float LIGHTS = 0.45;					
 const float LIGHTT = 0.15;					
 const float R = 0.35f;				// radius of car lights
 
 const vec3 LIGHTCOLOR		= vec3( 0.8, 0., 1. );		
 const vec3 SPECULARCOLOR 	= vec3( 1., 1., 1. );

 void
 main( )
 {
   
	vec3 newcolor = texture( uTexUnit, vST ).rgb;

	

 // now do the per-fragment lighting:

	vec3 Normal    = normalize(vN);
	vec3 Light     = normalize(vL);
	vec3 Eye       = normalize(vE);

	vec3 ambient = uKa * newcolor;

	float d = max( dot(Normal,Light), 0. );       // only do diffuse if the light can see the point
	vec3 diffuse = uKd * d * newcolor;

	float s = 0.;
	if( d > 0. )	          // only do specular if the light can see the point
	{
		vec3 ref = normalize(  reflect( -Light, Normal )  );
		float cosphi = dot( Eye, ref );
		if( cosphi > 0. )
			s = pow( max( cosphi, 0. ), uShininess );
	}
	vec3 specular = uKs * s * SPECULARCOLOR.rgb;
	gl_FragColor = vec4( ambient + diffuse + specular,  1. );
 }