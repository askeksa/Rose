const char *vshader = R"--(

uniform vec4 colors[256];

attribute vec4 xyuv;
attribute float tint;

varying vec2 uv;
varying vec4 color;

void main() {
	gl_Position = vec4(xyuv.xy, 0.0, 1.0);
	uv = xyuv.zw;
	color = colors[int(tint)];
}

)--";

const char *pshader = R"--(

varying vec2 uv;
varying vec4 color;

void main() {
	gl_FragColor = vec4(color.rgb, length(uv) < 1.0);
}

)--";
