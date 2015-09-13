const char *plot_vshader = R"--(

attribute vec4 xyuv;
attribute float tint;

varying vec2 uv;
varying vec4 color;

void main() {
	gl_Position = vec4(xyuv.xy, 0.0, 1.0);
	uv = xyuv.zw;
	if (tint >= 256.0) {
		color = vec4((511.0 - tint) / 255.0, 0, 0, 1);
	} else {
		color = vec4(tint / 255.0, 0, 0, 0);
	}
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

uniform vec4 colors[256];
uniform sampler2D image;

varying vec2 uv;

void main() {
	int tint = int(texture2D(image, uv).r * 255.0);
	gl_FragColor = vec4(colors[tint].rgb, 1.0);
}

)--";
