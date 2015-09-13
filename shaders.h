const char *plot_vshader = R"--(

uniform vec4 colors[512];

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

const char *plot_pshader = R"--(

varying vec2 uv;
varying vec4 color;

void main() {
	gl_FragColor = vec4(color.rgb, max(color.a, float(length(uv) < 1.0)));
}

)--";

const char *quad_vshader = R"--(

attribute vec2 xy;

varying vec2 uv;

void main() {
	gl_Position = vec4(xy, 0.0, 1.0);
	uv = (xy + 1.0) * 0.5;
}

)--";

const char *quad_pshader = R"--(

varying vec2 uv;

uniform sampler2D image;

void main() {
	gl_FragColor = texture2D(image, uv);
}

)--";
