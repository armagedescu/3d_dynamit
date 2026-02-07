//! # Dynamit RS
//!
//! A 3D graphics library with automatic shader generation.
//! Rust port of the C++ dynamit_gl library.
//!
//! ## Features
//! - **Automatic shader generation** - Based on buffer configuration
//! - **PolarBuilder** for parametric shape generation (cones, cylinders)
//! - **Mathematical formula-based geometry** via expression compiler
//! - **OpenGL rendering** via glow
//! - **Matrix and geometry utilities**
//!
//! ## Example - Using Dynamit (auto-generated shaders)
//! ```ignore
//! use dynamit_rs::{Dynamit, PolarBuilder};
//! use std::rc::Rc;
//!
//! // Generate geometry
//! let mut verts = Vec::new();
//! let mut norms = Vec::new();
//! let mut colors = Vec::new();
//! let mut indices = Vec::new();
//!
//! PolarBuilder::new()
//!     .formula("1 + 0.3 * sin(4 * theta)")
//!     .sectors(32)
//!     .color_outer(1.0, 0.5, 0.0, 1.0)
//!     .build_cone_indexed_with_color(&mut verts, &mut norms, &mut colors, &mut indices);
//!
//! // Create Dynamit and configure (shaders auto-generated)
//! let mut shape = Dynamit::new(gl)
//!     .with_vertices_3d(&verts)
//!     .with_normals_3d(&norms)
//!     .with_colors_4d(&colors)
//!     .with_indices(&indices)
//!     .with_transform_matrix_4f("mvp");
//!
//! // Render
//! shape.transform_matrix_4f(&mvp_matrix)?;
//! shape.draw_triangles_indexed()?;
//! ```

pub mod geometry;
pub mod builders;
pub mod shader_builder;
pub mod dynamit;

pub use calculator_3d;
pub use geometry::*;
pub use builders::PolarBuilder;
pub use shader_builder::{ShaderBuilder, ShaderSources, GlSet, StrideLayout};
pub use dynamit::Dynamit;
