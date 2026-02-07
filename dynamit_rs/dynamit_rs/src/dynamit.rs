//! Dynamit - Main rendering class with automatic shader generation.
//!
//! Port of C++ Dynamit class from Dynamit.h/cpp

use glow::HasContext;
use std::rc::Rc;

use crate::shader_builder::{
    BufferDesc, ConstColor, GlSet, LightDirection, ShaderBuilder, ShaderSources,
    StrideLayout, TransformMatrix4,
};

/// OpenGL buffer wrapper
struct GlBuffer {
    id: glow::Buffer,
    gl: Rc<glow::Context>,
}

impl GlBuffer {
    fn new(gl: Rc<glow::Context>) -> Self {
        let id = unsafe { gl.create_buffer().unwrap() };
        Self { id, gl }
    }
}

impl Drop for GlBuffer {
    fn drop(&mut self) {
        unsafe {
            self.gl.delete_buffer(self.id);
        }
    }
}

/// OpenGL VAO wrapper
struct GlVao {
    id: glow::VertexArray,
    gl: Rc<glow::Context>,
}

impl GlVao {
    fn new(gl: Rc<glow::Context>) -> Self {
        let id = unsafe { gl.create_vertex_array().unwrap() };
        Self { id, gl }
    }
}

impl Drop for GlVao {
    fn drop(&mut self) {
        unsafe {
            self.gl.delete_vertex_array(self.id);
        }
    }
}

/// OpenGL program wrapper
struct GlProgram {
    id: glow::Program,
    gl: Rc<glow::Context>,
}

impl GlProgram {
    fn new(gl: Rc<glow::Context>) -> Self {
        let id = unsafe { gl.create_program().unwrap() };
        Self { id, gl }
    }

    fn build(&self, vs_source: &str, fs_source: &str) -> Result<(), String> {
        unsafe {
            // Compile vertex shader
            let vs = self.gl.create_shader(glow::VERTEX_SHADER).unwrap();
            self.gl.shader_source(vs, vs_source);
            self.gl.compile_shader(vs);
            if !self.gl.get_shader_compile_status(vs) {
                let err = self.gl.get_shader_info_log(vs);
                self.gl.delete_shader(vs);
                return Err(format!("Vertex shader error: {}", err));
            }

            // Compile fragment shader
            let fs = self.gl.create_shader(glow::FRAGMENT_SHADER).unwrap();
            self.gl.shader_source(fs, fs_source);
            self.gl.compile_shader(fs);
            if !self.gl.get_shader_compile_status(fs) {
                let err = self.gl.get_shader_info_log(fs);
                self.gl.delete_shader(vs);
                self.gl.delete_shader(fs);
                return Err(format!("Fragment shader error: {}", err));
            }

            // Link program
            self.gl.attach_shader(self.id, vs);
            self.gl.attach_shader(self.id, fs);
            self.gl.link_program(self.id);

            self.gl.delete_shader(vs);
            self.gl.delete_shader(fs);

            if !self.gl.get_program_link_status(self.id) {
                return Err(format!("Program link error: {}", self.gl.get_program_info_log(self.id)));
            }

            Ok(())
        }
    }
}

impl Drop for GlProgram {
    fn drop(&mut self) {
        unsafe {
            self.gl.delete_program(self.id);
        }
    }
}

/// Per-VAO data container
struct VaoData {
    vao: GlVao,
    vertex_buffer: Option<GlBuffer>,
    normal_buffer: Option<GlBuffer>,
    color_buffer: Option<GlBuffer>,
    index_buffer: Option<GlBuffer>,
    stride_buffer: Option<GlBuffer>,
    vertex_count: usize,
    index_count: usize,
    index_type: u32,
    primitive_type: u32,
}

impl VaoData {
    fn new(gl: Rc<glow::Context>) -> Self {
        Self {
            vao: GlVao::new(gl),
            vertex_buffer: None,
            normal_buffer: None,
            color_buffer: None,
            index_buffer: None,
            stride_buffer: None,
            vertex_count: 0,
            index_count: 0,
            index_type: glow::UNSIGNED_INT,
            primitive_type: glow::TRIANGLES,
        }
    }
}

/// Dynamit - Main class for rendering with automatic shader generation
pub struct Dynamit {
    gl: Rc<glow::Context>,
    vao_list: Vec<VaoData>,
    current_vao_index: usize,
    program: Option<GlProgram>,
    program_built: bool,
    location_count: u32,

    // Shader configuration
    gl_set: GlSet,
    stride_layout: StrideLayout,
    stride_mode: bool,

    // Custom shader overrides
    custom_vertex_shader: Option<String>,
    custom_fragment_shader: Option<String>,

    // Uniform locations (cached after program build)
    transform_matrix_location: Option<glow::UniformLocation>,
    light_direction_location: Option<glow::UniformLocation>,
}

impl Dynamit {
    /// Create a new Dynamit instance
    pub fn new(gl: Rc<glow::Context>) -> Self {
        let mut d = Self {
            gl: gl.clone(),
            vao_list: Vec::new(),
            current_vao_index: 0,
            program: None,
            program_built: false,
            location_count: 0,
            gl_set: GlSet::new(),
            stride_layout: StrideLayout::new(),
            stride_mode: false,
            custom_vertex_shader: None,
            custom_fragment_shader: None,
            transform_matrix_location: None,
            light_direction_location: None,
        };
        d.vao_list.push(VaoData::new(gl));
        d
    }

    /// Access a specific VAO by index (creates if needed)
    pub fn vao(&mut self, index: usize) -> &mut Self {
        while index >= self.vao_list.len() {
            self.vao_list.push(VaoData::new(self.gl.clone()));
        }
        self.current_vao_index = index;
        self
    }

    /// Get VAO count
    pub fn vao_count(&self) -> usize {
        self.vao_list.len()
    }

    fn current_vao(&mut self) -> &mut VaoData {
        &mut self.vao_list[self.current_vao_index]
    }

    fn get_location_for(&mut self, name: &str) -> u32 {
        // Check stride layout first
        if self.stride_mode {
            if let Some(attr) = self.stride_layout.get_attribute(name) {
                return attr.location;
            }
        }

        // Check existing buffers
        if name == "vertex" {
            if let Some(vb) = &self.gl_set.vertex_buffer {
                return vb.location;
            }
        } else if name == "normal" {
            if let Some(nb) = &self.gl_set.normals_buffer {
                return nb.location;
            }
        } else if name == "color" {
            if let Some(cb) = &self.gl_set.colors_buffer {
                return cb.location;
            }
        }

        let loc = self.location_count;
        self.location_count += 1;
        loc
    }

    // ========================================
    // Fluent API - Vertices
    // ========================================

    /// Set 3D vertex data (separate buffer)
    pub fn with_vertices_3d(mut self, data: &[f32]) -> Self {
        let location = self.get_location_for("vertex");

        unsafe {
            self.gl.bind_vertex_array(Some(self.vao_list[self.current_vao_index].vao.id));

            let buffer = GlBuffer::new(self.gl.clone());
            self.gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer.id));
            self.gl.buffer_data_u8_slice(
                glow::ARRAY_BUFFER,
                bytemuck::cast_slice(data),
                glow::STATIC_DRAW,
            );
            self.gl.vertex_attrib_pointer_f32(location, 3, glow::FLOAT, false, 0, 0);
            self.gl.enable_vertex_attrib_array(location);

            let vao = self.current_vao();
            vao.vertex_buffer = Some(buffer);
            vao.vertex_count = data.len() / 3;
        }

        self.gl_set.vertex_buffer = Some(BufferDesc {
            name: "vertex".to_string(),
            location,
            dimension: 3,
        });

        self
    }

    /// Set 3D normal data (separate buffer)
    pub fn with_normals_3d(mut self, data: &[f32]) -> Self {
        let location = self.get_location_for("normal");

        unsafe {
            self.gl.bind_vertex_array(Some(self.vao_list[self.current_vao_index].vao.id));

            let buffer = GlBuffer::new(self.gl.clone());
            self.gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer.id));
            self.gl.buffer_data_u8_slice(
                glow::ARRAY_BUFFER,
                bytemuck::cast_slice(data),
                glow::STATIC_DRAW,
            );
            self.gl.vertex_attrib_pointer_f32(location, 3, glow::FLOAT, false, 0, 0);
            self.gl.enable_vertex_attrib_array(location);

            self.current_vao().normal_buffer = Some(buffer);
        }

        self.gl_set.normals_buffer = Some(BufferDesc {
            name: "normal".to_string(),
            location,
            dimension: 3,
        });

        self
    }

    /// Set 4D color data (separate buffer)
    pub fn with_colors_4d(mut self, data: &[f32]) -> Self {
        let location = self.get_location_for("color");

        unsafe {
            self.gl.bind_vertex_array(Some(self.vao_list[self.current_vao_index].vao.id));

            let buffer = GlBuffer::new(self.gl.clone());
            self.gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer.id));
            self.gl.buffer_data_u8_slice(
                glow::ARRAY_BUFFER,
                bytemuck::cast_slice(data),
                glow::STATIC_DRAW,
            );
            self.gl.vertex_attrib_pointer_f32(location, 4, glow::FLOAT, false, 0, 0);
            self.gl.enable_vertex_attrib_array(location);

            self.current_vao().color_buffer = Some(buffer);
        }

        self.gl_set.colors_buffer = Some(BufferDesc {
            name: "color".to_string(),
            location,
            dimension: 4,
        });

        self
    }

    /// Set index buffer (u32)
    pub fn with_indices(mut self, indices: &[u32]) -> Self {
        unsafe {
            self.gl.bind_vertex_array(Some(self.vao_list[self.current_vao_index].vao.id));

            let buffer = GlBuffer::new(self.gl.clone());
            self.gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, Some(buffer.id));
            self.gl.buffer_data_u8_slice(
                glow::ELEMENT_ARRAY_BUFFER,
                bytemuck::cast_slice(indices),
                glow::STATIC_DRAW,
            );

            let vao = self.current_vao();
            vao.index_buffer = Some(buffer);
            vao.index_count = indices.len();
            vao.index_type = glow::UNSIGNED_INT;
        }

        self
    }

    // ========================================
    // Fluent API - Stride mode
    // ========================================

    /// Set interleaved vertex data
    pub fn with_stride(mut self, data: &[f32], stride_bytes: i32) -> Self {
        self.stride_layout.set_stride(stride_bytes);
        self.stride_mode = true;

        unsafe {
            self.gl.bind_vertex_array(Some(self.vao_list[self.current_vao_index].vao.id));

            let buffer = GlBuffer::new(self.gl.clone());
            self.gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer.id));
            self.gl.buffer_data_u8_slice(
                glow::ARRAY_BUFFER,
                bytemuck::cast_slice(data),
                glow::STATIC_DRAW,
            );

            let vao = self.current_vao();
            vao.stride_buffer = Some(buffer);
            vao.vertex_count = (data.len() * 4) / stride_bytes as usize;
        }

        self
    }

    /// Add vertex attribute to stride layout
    pub fn with_stride_vertices(mut self, size: i32) -> Self {
        let location = self.get_location_for("vertex");
        self.stride_layout.add_attribute("vertex", location, size, false);
        self
    }

    /// Add normal attribute to stride layout
    pub fn with_stride_normals(mut self, size: i32) -> Self {
        let location = self.get_location_for("normal");
        self.stride_layout.add_attribute("normal", location, size, false);
        self
    }

    /// Add color attribute to stride layout
    pub fn with_stride_colors(mut self, size: i32) -> Self {
        let location = self.get_location_for("color");
        self.stride_layout.add_attribute("color", location, size, false);
        self
    }

    // ========================================
    // Fluent API - Constants and uniforms
    // ========================================

    /// Set constant color
    pub fn with_const_color(mut self, r: f32, g: f32, b: f32, a: f32) -> Self {
        self.gl_set.const_color = Some(ConstColor {
            name: "constColor".to_string(),
            data: [r, g, b, a],
        });
        self
    }

    /// Set constant light direction
    pub fn with_const_light_direction(mut self, x: f32, y: f32, z: f32) -> Self {
        self.gl_set.light_direction = Some(LightDirection {
            name: "lightDirection".to_string(),
            data: [x, y, z],
            normalize: true,
            is_const: true,
        });
        self
    }

    /// Enable transform matrix uniform
    pub fn with_transform_matrix_4f(mut self, name: &str) -> Self {
        self.gl_set.transform_matrix4 = Some(TransformMatrix4 {
            name: name.to_string(),
            ..Default::default()
        });
        self
    }

    /// Set primitive type
    pub fn with_primitive(mut self, primitive: u32) -> Self {
        self.vao_list[self.current_vao_index].primitive_type = primitive;
        self
    }

    /// Set custom shaders (bypass auto-generation)
    pub fn with_shader_sources(mut self, vs: &str, fs: &str) -> Self {
        self.custom_vertex_shader = Some(vs.to_string());
        self.custom_fragment_shader = Some(fs.to_string());
        self
    }

    // ========================================
    // Program building
    // ========================================

    /// Build the shader program (auto-generates if no custom shaders)
    pub fn build_program(&mut self) -> Result<(), String> {
        if self.program_built {
            return Ok(());
        }

        // Finalize stride layout if in stride mode
        if self.stride_mode {
            self.stride_layout.finalize();
            self.apply_stride_layout()?;
        }

        let sources = if self.custom_vertex_shader.is_some() && self.custom_fragment_shader.is_some() {
            ShaderSources {
                vertex_shader: self.custom_vertex_shader.clone().unwrap(),
                fragment_shader: self.custom_fragment_shader.clone().unwrap(),
            }
        } else {
            let mut builder = ShaderBuilder::new();
            if self.stride_mode {
                builder.build_from_stride(&self.gl_set, &self.stride_layout)
            } else {
                builder.build_from_glset(&self.gl_set)
            }
        };

        let program = GlProgram::new(self.gl.clone());
        program.build(&sources.vertex_shader, &sources.fragment_shader)?;
        self.program = Some(program);
        self.program_built = true;

        Ok(())
    }

    fn apply_stride_layout(&mut self) -> Result<(), String> {
        let stride = self.stride_layout.stride();
        if stride == 0 {
            return Ok(());
        }

        for vao_data in &self.vao_list {
            if let Some(ref buffer) = vao_data.stride_buffer {
                unsafe {
                    self.gl.bind_vertex_array(Some(vao_data.vao.id));
                    self.gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer.id));

                    for attr in self.stride_layout.attributes() {
                        self.gl.vertex_attrib_pointer_f32(
                            attr.location,
                            attr.size,
                            glow::FLOAT,
                            attr.normalized,
                            stride,
                            attr.offset,
                        );
                        self.gl.enable_vertex_attrib_array(attr.location);
                    }
                }
            }
        }

        Ok(())
    }

    /// Get the generated shader sources (for debugging)
    pub fn get_generated_shaders(&self) -> ShaderSources {
        let mut builder = ShaderBuilder::new();
        if self.stride_mode {
            builder.build_from_stride(&self.gl_set, &self.stride_layout)
        } else {
            builder.build_from_glset(&self.gl_set)
        }
    }

    /// Log the generated shaders to stdout
    pub fn log_generated_shaders(&self) {
        let sources = self.get_generated_shaders();
        println!("=== VERTEX SHADER ===\n{}", sources.vertex_shader);
        println!("=== FRAGMENT SHADER ===\n{}", sources.fragment_shader);
    }

    // ========================================
    // Binding and drawing
    // ========================================

    /// Use the program
    pub fn use_program(&mut self) -> Result<(), String> {
        self.build_program()?;
        if let Some(ref program) = self.program {
            unsafe {
                self.gl.use_program(Some(program.id));
            }
        }
        Ok(())
    }

    /// Bind VAO
    pub fn bind_vertex_array(&self, index: usize) {
        if index < self.vao_list.len() {
            unsafe {
                self.gl.bind_vertex_array(Some(self.vao_list[index].vao.id));
            }
        }
    }

    /// Bind program and VAO
    pub fn bind(&mut self) -> Result<(), String> {
        self.use_program()?;
        self.bind_vertex_array(0);
        Ok(())
    }

    /// Draw triangles (non-indexed)
    pub fn draw_triangles(&mut self) -> Result<(), String> {
        self.use_program()?;

        for vao_data in &self.vao_list {
            if vao_data.vertex_count > 0 {
                unsafe {
                    self.gl.bind_vertex_array(Some(vao_data.vao.id));
                    self.gl.draw_arrays(vao_data.primitive_type, 0, vao_data.vertex_count as i32);
                }
            }
        }

        Ok(())
    }

    /// Draw triangles (indexed)
    pub fn draw_triangles_indexed(&mut self) -> Result<(), String> {
        self.use_program()?;

        for vao_data in &self.vao_list {
            if vao_data.index_count > 0 {
                unsafe {
                    self.gl.bind_vertex_array(Some(vao_data.vao.id));
                    self.gl.draw_elements(
                        vao_data.primitive_type,
                        vao_data.index_count as i32,
                        vao_data.index_type,
                        0,
                    );
                }
            }
        }

        Ok(())
    }

    // ========================================
    // Runtime uniform updates
    // ========================================

    /// Update transform matrix uniform
    pub fn transform_matrix_4f(&mut self, data: &[f32; 16]) -> Result<(), String> {
        self.use_program()?;

        let program = self.program.as_ref().ok_or("Program not built")?;
        let tm = self.gl_set.transform_matrix4.as_ref().ok_or("Transform matrix not configured")?;

        let location = if let Some(loc) = &self.transform_matrix_location {
            loc.clone()
        } else {
            let loc = unsafe { self.gl.get_uniform_location(program.id, &tm.name) };
            self.transform_matrix_location = loc.clone();
            loc.ok_or("Transform matrix uniform not found in shader")?
        };

        unsafe {
            self.gl.uniform_matrix_4_f32_slice(Some(&location), false, data);
        }

        Ok(())
    }
}
