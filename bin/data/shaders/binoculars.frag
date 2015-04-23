#version 120

//void main(){
//	//this is the fragment shader
//	//this is where the pixel level drawing happens
//	//gl_FragCoord gives us the x and y of the current pixel its drawing
	
//	//we grab the x and y and store them in an int
//	float xVal = gl_FragCoord.x;
//	float yVal = gl_FragCoord.y;
	
//	//we use the mod function to only draw pixels if they are every 2 in x or every 4 in y
//	if( mod(xVal, 2.0) == 0.5 && mod(yVal, 2.0) == 0.5 ){
//		gl_FragColor = gl_Color;    
//    }else{
//		gl_FragColor.a = 0.5;
//	}
	
//}

//uniform float border; // 0.01
//uniform float circle_radius; // 0.5
//uniform vec4 circle_color; // vec4(1.0, 1.0, 1.0, 1.0)
//uniform vec2 circle_center; // vec2(0.5, 0.5)    
uniform float alpha;
uniform bool use_binocular;
uniform float aspect_ratio;
void main (void)
{
	float border = 0.01;
	float offset = 0.01;
	float circle_radius = (0.5/aspect_ratio)-offset;
	vec4 outer_color = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 inner_color = gl_FragColor;
	vec2 circle_center = vec2(circle_radius+offset, 0.5 / aspect_ratio);
	vec2 circle_center2 = vec2(1 - circle_radius-offset, 0.5 / aspect_ratio);
	float xVal = gl_TexCoord[0].x;
	float yVal = gl_TexCoord[0].y / aspect_ratio;
  
  // Offset uv with the center of the circle.
  float dist1 =  sqrt((circle_center.x - xVal)*(circle_center.x - xVal) + (circle_center.y - yVal)*(circle_center.y - yVal));
  float dist2 =  sqrt((circle_center2.x - xVal)*(circle_center2.x - xVal) + (circle_center2.y - yVal)*(circle_center2.y - yVal));
  float dist = min(dist1,dist2);
  float t = smoothstep(circle_radius+border, circle_radius-border, dist);
  if ( dist < circle_radius+border )
  {
    inner_color.a = alpha;
  }
   
  vec4 col = mix(outer_color, inner_color, t);


	/*
  else
  { 
    gl_FragColor = outer_color;
  }*/
  
  if (use_binocular) {
	gl_FragColor = col;
	}
	else {
	inner_color.a = alpha;
	gl_FragColor = inner_color;
	}
}