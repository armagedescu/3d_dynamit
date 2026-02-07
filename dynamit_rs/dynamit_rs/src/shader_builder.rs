//! Shader builder - automatic GLSL generation based on buffer configuration.
//!
//! Port of C++ ShaderStrategy and related classes from Dynamit.h/cpp

use std::fmt::Write;

/// Vertex attribute description for stride layouts
#[derive(Debug, Clone)]
pub struct StrideAttribute {
    pub name: String,
    pub location: u32,
    pub size: i32,      // Component count (2, 3, 4)
    pub normalized: bool,
    pub offset: i32,    // Byte offset within stride
}

/// Describes interleaved buffer layout
#[derive(Debug, Clone, Default)]
pub struct StrideLayout {
    attributes: Vec<StrideAttribute>,
    total_stride: i32,
    current_offset: i32,
}

impl StrideLayout {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn add_attribute(&mut self, name: &str, location: u32, size: i32, normalized: bool) {
        let attr = StrideAttribute {
            name: name.to_string(),
            location,
            size,
            normalized,
            offset: self.current_offset,
        };
        self.attributes.push(attr);
        self.current_offset += size * 4; // sizeof(f32)
    }

    pub fn set_offset(&mut self, offset: i32) {
        self.current_offset = offset;
    }

    pub fn set_stride(&mut self, stride: i32) {
        self.total_stride = stride;
    }

    pub fn finalize(&mut self) {
        if self.total_stride == 0 {
            self.total_stride = self.current_offset;
        }
    }

    pub fn attributes(&self) -> &[StrideAttribute] {
        &self.attributes
    }

    pub fn stride(&self) -> i32 {
        self.total_stride
    }

    pub fn has_attribute(&self, name: &str) -> bool {
        self.attributes.iter().any(|a| a.name == name)
    }

    pub fn get_attribute(&self, name: &str) -> Option<&StrideAttribute> {
        self.attributes.iter().find(|a| a.name == name)
    }
}

/// Light direction configuration
#[derive(Debug, Clone)]
pub struct LightDirection {
    pub name: String,
    pub data: [f32; 3],
    pub normalize: bool,
    pub is_const: bool,
}

impl Default for LightDirection {
    fn default() -> Self {
        Self {
            name: "lightDirection".to_string(),
            data: [0.0, 0.5, 1.0],
            normalize: true,
            is_const: true,
        }
    }
}

impl LightDirection {
    pub fn to_glsl_const(&self) -> String {
        format!(
            "const vec3 {} = vec3({}, {}, {});",
            self.name, self.data[0], self.data[1], self.data[2]
        )
    }

    pub fn to_glsl_uniform(&self) -> String {
        format!("uniform vec3 {};", self.name)
    }
}

/// Constant color configuration
#[derive(Debug, Clone)]
pub struct ConstColor {
    pub name: String,
    pub data: [f32; 4],
}

impl Default for ConstColor {
    fn default() -> Self {
        Self {
            name: "constColor".to_string(),
            data: [0.7, 0.7, 0.7, 1.0],
        }
    }
}

impl ConstColor {
    pub fn to_glsl(&self) -> String {
        format!(
            "const vec4 {} = vec4({}, {}, {}, {});",
            self.name, self.data[0], self.data[1], self.data[2], self.data[3]
        )
    }
}

/// Transform matrix configuration
#[derive(Debug, Clone)]
pub struct TransformMatrix4 {
    pub name: String,
    pub data: [f32; 16],
}

impl Default for TransformMatrix4 {
    fn default() -> Self {
        Self {
            name: "transformMatrix".to_string(),
            data: [
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0,
            ],
        }
    }
}

impl TransformMatrix4 {
    pub fn to_glsl_uniform(&self) -> String {
        format!("uniform mat4 {};", self.name)
    }
}

/// Buffer description for shader generation
#[derive(Debug, Clone)]
pub struct BufferDesc {
    pub name: String,
    pub location: u32,
    pub dimension: i32,
}

impl BufferDesc {
    pub fn decl(&self) -> String {
        format!("vec{} {}", self.dimension, self.name)
    }

    pub fn name_vary(&self) -> String {
        format!("{}Vary", self.name)
    }

    pub fn decl_vary(&self) -> String {
        format!("vec{} {}", self.dimension, self.name_vary())
    }

    pub fn default_vary_assign(&self) -> String {
        format!("{} = {}", self.name_vary(), self.name)
    }

    pub fn vec4(&self, default_w: f32) -> String {
        if self.dimension == 4 {
            return self.name.clone();
        }

        let mut s = format!("vec4({}", self.name);
        for _ in self.dimension..3 {
            s.push_str(", 0.0");
        }
        if self.dimension < 4 {
            write!(s, ", {}", default_w).unwrap();
        }
        s.push(')');
        s
    }

    pub fn vec4_vary(&self, default_w: f32) -> String {
        if self.dimension == 4 {
            return self.name_vary();
        }

        let mut s = format!("vec4({}", self.name_vary());
        for _ in self.dimension..3 {
            s.push_str(", 0.0");
        }
        if self.dimension < 4 {
            write!(s, ", {}", default_w).unwrap();
        }
        s.push(')');
        s
    }
}

/// Rendering state for shader generation
#[derive(Debug, Clone, Default)]
pub struct GlSet {
    pub precision: String,
    pub vertex_buffer: Option<BufferDesc>,
    pub normals_buffer: Option<BufferDesc>,
    pub colors_buffer: Option<BufferDesc>,
    pub const_color: Option<ConstColor>,
    pub light_direction: Option<LightDirection>,
    pub transform_matrix4: Option<TransformMatrix4>,
}

impl GlSet {
    pub fn new() -> Self {
        Self {
            precision: "mediump float".to_string(),
            ..Default::default()
        }
    }

    pub fn require_color(&mut self) {
        if self.colors_buffer.is_none() && self.const_color.is_none() {
            self.const_color = Some(ConstColor::default());
        }
    }

    pub fn require_light_direction(&mut self) {
        if self.light_direction.is_none() {
            self.light_direction = Some(LightDirection::default());
        }
    }
}

/// Generated shader sources
#[derive(Debug, Clone)]
pub struct ShaderSources {
    pub vertex_shader: String,
    pub fragment_shader: String,
}

/// Shader builder that implements automatic GLSL generation
pub struct ShaderBuilder {
    vs_head: Vec<String>,
    vs_main: Vec<String>,
    fs_head: Vec<String>,
    fs_main: Vec<String>,
}

impl ShaderBuilder {
    pub fn new() -> Self {
        Self {
            vs_head: Vec::new(),
            vs_main: Vec::new(),
            fs_head: Vec::new(),
            fs_main: Vec::new(),
        }
    }

    pub fn reset(&mut self) {
        self.vs_head.clear();
        self.vs_main.clear();
        self.fs_head.clear();
        self.fs_main.clear();
    }

    /// Build shaders from GlSet configuration (separate buffers mode)
    pub fn build_from_glset(&mut self, gl_set: &GlSet) -> ShaderSources {
        self.reset();

        // Set requirements
        let mut gl_set = gl_set.clone();
        gl_set.require_color();
        if gl_set.normals_buffer.is_some() {
            gl_set.require_light_direction();
        }

        self.add_declarations(&gl_set);
        self.compose_vertex_main(&gl_set, None);
        self.compose_fragment_main(&gl_set, None);

        ShaderSources {
            vertex_shader: self.build_vertex_shader(),
            fragment_shader: self.build_fragment_shader(),
        }
    }

    /// Build shaders from stride layout configuration
    pub fn build_from_stride(&mut self, gl_set: &GlSet, layout: &StrideLayout) -> ShaderSources {
        self.reset();

        // Set requirements
        let mut gl_set = gl_set.clone();
        gl_set.require_color();
        if layout.has_attribute("normal") || gl_set.normals_buffer.is_some() {
            gl_set.require_light_direction();
        }

        self.add_stride_declarations(&gl_set, layout);
        self.compose_vertex_main(&gl_set, Some(layout));
        self.compose_fragment_main(&gl_set, Some(layout));

        ShaderSources {
            vertex_shader: self.build_vertex_shader(),
            fragment_shader: self.build_fragment_shader(),
        }
    }

    fn add_declarations(&mut self, gl_set: &GlSet) {
        self.fs_head.push(format!("precision {};", gl_set.precision));
        self.fs_head.push("out vec4 fragColor;".to_string());

        if let Some(vb) = &gl_set.vertex_buffer {
            self.vs_head.push(format!(
                "layout (location = {}) in {};",
                vb.location, vb.decl()
            ));
        }

        if let Some(nb) = &gl_set.normals_buffer {
            self.vs_head.push(format!(
                "layout (location = {}) in {};",
                nb.location, nb.decl()
            ));
            self.vs_head.push(format!("out {};", nb.decl_vary()));
            self.fs_head.push(format!("in {};", nb.decl_vary()));
        }

        if let Some(cb) = &gl_set.colors_buffer {
            self.vs_head.push(format!(
                "layout (location = {}) in {};",
                cb.location, cb.decl()
            ));
            self.vs_head.push(format!("out {};", cb.decl_vary()));
            self.fs_head.push(format!("in {};", cb.decl_vary()));
        }

        if let Some(cc) = &gl_set.const_color {
            self.fs_head.push(cc.to_glsl());
        }

        if let Some(light) = &gl_set.light_direction {
            if light.is_const {
                self.fs_head.push(light.to_glsl_const());
            } else {
                self.fs_head.push(light.to_glsl_uniform());
            }
        }

        if let Some(tm) = &gl_set.transform_matrix4 {
            self.vs_head.push(tm.to_glsl_uniform());
        }
    }

    fn add_stride_declarations(&mut self, gl_set: &GlSet, layout: &StrideLayout) {
        self.fs_head.push(format!("precision {};", gl_set.precision));
        self.fs_head.push("out vec4 fragColor;".to_string());

        for attr in layout.attributes() {
            let vec_type = format!("vec{}", attr.size);
            self.vs_head.push(format!(
                "layout (location = {}) in {} {};",
                attr.location, vec_type, attr.name
            ));

            // Add varyings for normals and colors
            if attr.name == "normal" || attr.name == "color" {
                self.vs_head.push(format!("out {} {}Vary;", vec_type, attr.name));
                self.fs_head.push(format!("in {} {}Vary;", vec_type, attr.name));
            }
        }

        if let Some(cc) = &gl_set.const_color {
            self.fs_head.push(cc.to_glsl());
        }

        if let Some(light) = &gl_set.light_direction {
            if light.is_const {
                self.fs_head.push(light.to_glsl_const());
            } else {
                self.fs_head.push(light.to_glsl_uniform());
            }
        }

        if let Some(tm) = &gl_set.transform_matrix4 {
            self.vs_head.push(tm.to_glsl_uniform());
        }
    }

    fn build_position_expression(&self, gl_set: &GlSet, layout: Option<&StrideLayout>) -> String {
        let (vertex_expr, vertex_dim) = if let Some(layout) = layout {
            if let Some(attr) = layout.get_attribute("vertex") {
                ("vertex".to_string(), attr.size)
            } else if let Some(vb) = &gl_set.vertex_buffer {
                (vb.name.clone(), vb.dimension)
            } else {
                ("vec3(0.0, 0.0, 0.0)".to_string(), 3)
            }
        } else if let Some(vb) = &gl_set.vertex_buffer {
            (vb.name.clone(), vb.dimension)
        } else {
            ("vec3(0.0, 0.0, 0.0)".to_string(), 3)
        };

        if let Some(tm) = &gl_set.transform_matrix4 {
            if vertex_dim == 4 {
                format!("{} * {}", tm.name, vertex_expr)
            } else if vertex_dim == 3 {
                format!("{} * vec4({}, 1.0)", tm.name, vertex_expr)
            } else {
                format!("{} * vec4({}, 0.0, 1.0)", tm.name, vertex_expr)
            }
        } else if vertex_dim == 4 {
            vertex_expr
        } else if vertex_dim == 3 {
            format!("vec4({}, 1.0)", vertex_expr)
        } else {
            format!("vec4({}, 0.0, 1.0)", vertex_expr)
        }
    }

    fn build_color_expression(&self, gl_set: &GlSet, layout: Option<&StrideLayout>) -> String {
        if let Some(layout) = layout {
            if let Some(attr) = layout.get_attribute("color") {
                return if attr.size == 4 {
                    "colorVary".to_string()
                } else {
                    "vec4(colorVary, 1.0)".to_string()
                };
            }
        }

        if let Some(cb) = &gl_set.colors_buffer {
            return cb.vec4_vary(1.0);
        }

        if let Some(cc) = &gl_set.const_color {
            return cc.name.clone();
        }

        "vec4(1.0, 1.0, 1.0, 1.0)".to_string()
    }

    fn build_lighting_factor(&self, gl_set: &GlSet, layout: Option<&StrideLayout>) -> Option<String> {
        let has_normals = layout.map_or(false, |l| l.has_attribute("normal"))
            || gl_set.normals_buffer.is_some();

        if !has_normals {
            return None;
        }

        let light = gl_set.light_direction.as_ref()?;
        let normal_var = "normalVary";

        Some(if light.normalize {
            format!("-dot(normalize({}), normalize({}))", light.name, normal_var)
        } else {
            format!("-dot({}, {})", light.name, normal_var)
        })
    }

    fn compose_vertex_main(&mut self, gl_set: &GlSet, layout: Option<&StrideLayout>) {
        let pos_expr = self.build_position_expression(gl_set, layout);
        self.vs_main.push(format!("gl_Position = {};", pos_expr));

        if let Some(layout) = layout {
            if layout.has_attribute("normal") {
                if let Some(tm) = &gl_set.transform_matrix4 {
                    self.vs_main.push(format!("normalVary = mat3({}) * normal;", tm.name));
                } else {
                    self.vs_main.push("normalVary = normal;".to_string());
                }
            }
            if layout.has_attribute("color") {
                self.vs_main.push("colorVary = color;".to_string());
            }
        } else {
            if let Some(nb) = &gl_set.normals_buffer {
                if let Some(tm) = &gl_set.transform_matrix4 {
                    self.vs_main.push(format!(
                        "{} = mat3({}) * {};",
                        nb.name_vary(), tm.name, nb.name
                    ));
                } else {
                    self.vs_main.push(format!("{};", nb.default_vary_assign()));
                }
            }
            if let Some(cb) = &gl_set.colors_buffer {
                self.vs_main.push(format!("{};", cb.default_vary_assign()));
            }
        }
    }

    fn compose_fragment_main(&mut self, gl_set: &GlSet, layout: Option<&StrideLayout>) {
        let color_expr = self.build_color_expression(gl_set, layout);

        if let Some(lighting) = self.build_lighting_factor(gl_set, layout) {
            self.fs_main.push(format!("float prod = {};", lighting));
            self.fs_main.push(format!("fragColor = vec4({}.rgb * prod, 1.0);", color_expr));
        } else {
            self.fs_main.push(format!("fragColor = {};", color_expr));
        }
    }

    fn build_vertex_shader(&self) -> String {
        let mut s = String::from("#version 330 core\n");
        for line in &self.vs_head {
            s.push_str(line);
            s.push('\n');
        }
        s.push_str("\nvoid main()\n{\n");
        for line in &self.vs_main {
            s.push_str("    ");
            s.push_str(line);
            s.push('\n');
        }
        s.push_str("}\n");
        s
    }

    fn build_fragment_shader(&self) -> String {
        let mut s = String::from("#version 330 core\n");
        for line in &self.fs_head {
            s.push_str(line);
            s.push('\n');
        }
        s.push_str("\nvoid main()\n{\n");
        for line in &self.fs_main {
            s.push_str("    ");
            s.push_str(line);
            s.push('\n');
        }
        s.push_str("}\n");
        s
    }
}

impl Default for ShaderBuilder {
    fn default() -> Self {
        Self::new()
    }
}
