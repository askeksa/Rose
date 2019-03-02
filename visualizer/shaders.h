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

uniform bool enable_overlay;
uniform float cpu_compute_cycles;
uniform float cpu_draw_cycles;
uniform float copper_cycles;
uniform float blitter_cycles;

const float CYCLES_PER_FRAME = 139598.0;
const float BAR_SOLID = 2.0;
const float BAR_MAX = 2.5;

varying vec2 uv;

void main() {
	int tint = int(texture2D(image, uv).r * 255.0);
	vec3 color = colors[tint].rgb;
	if (enable_overlay) {
		float x = uv.x;
		float y = uv.y;
		if (x > 0.79 && x < 0.95 && y > 0.05 && y < 0.95) {
			color = color * 0.5;
		}
		float b = (y - 0.1) / 0.8;
		if (b > 0.0 && b < 1.0) {
			float v = b * BAR_MAX;
			float c = v * CYCLES_PER_FRAME;
			vec3 bar = color;
			if (x > 0.84 && x < 0.85) {
				// CPU bar
				float c1 = cpu_compute_cycles;
				float c2 = c1 + cpu_draw_cycles;
				if (c < c1) bar = vec3(0.0, 0.5, 0.5);
				else if (c < c2) bar = vec3(0.5, 1.0, 0.5);
			}
			if (x > 0.89 && x < 0.9) {
				// Copper / blitter bar
				float c1 = copper_cycles;
				float c2 = c1 + blitter_cycles;
				if (c < c1) bar = vec3(0.8, 0.5, 0.2);
				else if (c < c2) bar = vec3(1.0, 1.0, 0.5);
			}
			float opacity = v < BAR_SOLID ? 1.0 : (BAR_MAX - v) / (BAR_MAX - BAR_SOLID);
			color = mix(color, bar, opacity);
			if (x > 0.83 && x < 0.91 && v > 0.99 && v < 1.0) color = vec3(1);
		}
	}
	gl_FragColor = vec4(color, 1.0);
}

)--";
