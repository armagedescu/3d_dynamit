//! Shape builders - PolarBuilder for parametric geometry generation.
//!
//! Port of C++ builders.h/cpp

#![allow(unused_assignments)] // theta_val is read through bound pointer in expr.eval()

use std::f32::consts::PI;
use calculator_3d::ExpressionCompiler;

/// Builder for polar/cylindrical coordinate based shapes
#[derive(Clone)]
pub struct PolarBuilder {
    formula: String,
    domain_start: f32,
    domain_end: f32,
    sectors: usize,
    slices: usize,
    turbo: bool,
    smooth: bool,
    double_coated: bool,
    reversed: bool,
    color_outer: [f32; 4],
    color_inner: [f32; 4],
}

impl Default for PolarBuilder {
    fn default() -> Self {
        Self::new()
    }
}

impl PolarBuilder {
    /// Create a new PolarBuilder with default settings
    pub fn new() -> Self {
        Self {
            formula: "1".to_string(),
            domain_start: 0.0,
            domain_end: 2.0 * PI,
            sectors: 8,
            slices: 4,
            turbo: true,
            smooth: true,
            double_coated: false,
            reversed: false,
            color_outer: [1.0, 1.0, 1.0, 1.0],
            color_inner: [1.0, 1.0, 1.0, 1.0],
        }
    }

    /// Set the radius formula (function of theta)
    pub fn formula(mut self, formula: &str) -> Self {
        self.formula = formula.to_string();
        self
    }

    /// Set the angular domain
    pub fn domain(mut self, start: f32, end: f32) -> Self {
        self.domain_start = start;
        self.domain_end = end;
        self
    }

    /// Set just the domain end (start = 0)
    pub fn domain_end(mut self, end: f32) -> Self {
        self.domain_end = end;
        self
    }

    /// Set number of angular sectors
    pub fn sectors(mut self, n: usize) -> Self {
        self.sectors = n.max(3);
        self
    }

    /// Set number of vertical slices
    pub fn slices(mut self, n: usize) -> Self {
        self.slices = n.max(1);
        self
    }

    /// Set both sectors and slices
    pub fn sectors_slices(mut self, sectors: usize, slices: usize) -> Self {
        self.sectors = sectors.max(3);
        self.slices = slices.max(1);
        self
    }

    /// Enable/disable turbo mode (optimized generation)
    pub fn turbo(mut self, enabled: bool) -> Self {
        self.turbo = enabled;
        self
    }

    /// Enable/disable smooth normals
    pub fn smooth(mut self, enabled: bool) -> Self {
        self.smooth = enabled;
        self
    }

    /// Alias for smooth(false)
    pub fn edged(mut self, enabled: bool) -> Self {
        self.smooth = !enabled;
        self
    }

    /// Enable/disable double-coated (both sides rendered)
    pub fn double_coated(mut self, enabled: bool) -> Self {
        self.double_coated = enabled;
        self
    }

    /// Alias for double_coated(false)
    pub fn single_coated(mut self, enabled: bool) -> Self {
        self.double_coated = !enabled;
        self
    }

    /// Enable/disable reversed normals
    pub fn reversed(mut self, enabled: bool) -> Self {
        self.reversed = enabled;
        self
    }

    /// Set outer color
    pub fn color_outer(mut self, r: f32, g: f32, b: f32, a: f32) -> Self {
        self.color_outer = [r, g, b, a];
        self
    }

    /// Set inner color
    pub fn color_inner(mut self, r: f32, g: f32, b: f32, a: f32) -> Self {
        self.color_inner = [r, g, b, a];
        self
    }

    /// Set both colors
    pub fn color(mut self, rgba: [f32; 4]) -> Self {
        self.color_outer = rgba;
        self.color_inner = rgba;
        self
    }

    /// Set both colors separately
    pub fn colors(mut self, outer: [f32; 4], inner: [f32; 4]) -> Self {
        self.color_outer = outer;
        self.color_inner = inner;
        self
    }

    /// Build a cone shape (indexed)
    pub fn build_cone_indexed(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        indices: &mut Vec<u32>,
    ) -> Result<(), String> {
        self.build_cone_indexed_internal(verts, norms, indices, false)?;
        if self.double_coated {
            self.build_cone_indexed_internal(verts, norms, indices, true)?;
        }
        Ok(())
    }

    /// Build a cone shape with colors (indexed)
    pub fn build_cone_indexed_with_color(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        colors: &mut Vec<f32>,
        indices: &mut Vec<u32>,
    ) -> Result<(), String> {
        self.build_cone_indexed_with_color_internal(verts, norms, colors, indices, false)?;
        if self.double_coated {
            self.build_cone_indexed_with_color_internal(verts, norms, colors, indices, true)?;
        }
        Ok(())
    }

    /// Build a cylinder shape (indexed)
    pub fn build_cylinder_indexed(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        indices: &mut Vec<u32>,
    ) -> Result<(), String> {
        self.build_cylinder_indexed_internal(verts, norms, indices, false)?;
        if self.double_coated {
            self.build_cylinder_indexed_internal(verts, norms, indices, true)?;
        }
        Ok(())
    }

    /// Build a cylinder shape with colors (indexed)
    pub fn build_cylinder_indexed_with_color(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        colors: &mut Vec<f32>,
        indices: &mut Vec<u32>,
    ) -> Result<(), String> {
        self.build_cylinder_indexed_with_color_internal(verts, norms, colors, indices, false)?;
        if self.double_coated {
            self.build_cylinder_indexed_with_color_internal(verts, norms, colors, indices, true)?;
        }
        Ok(())
    }

    /// Rebuild a cone (clears buffers first)
    pub fn rebuild_cone_indexed(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        indices: &mut Vec<u32>,
    ) -> Result<(), String> {
        verts.clear();
        norms.clear();
        indices.clear();
        self.build_cone_indexed(verts, norms, indices)
    }

    /// Rebuild a cylinder (clears buffers first)
    pub fn rebuild_cylinder_indexed(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        indices: &mut Vec<u32>,
    ) -> Result<(), String> {
        verts.clear();
        norms.clear();
        indices.clear();
        self.build_cylinder_indexed(verts, norms, indices)
    }

    // Internal cone builder
    fn build_cone_indexed_internal(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        indices: &mut Vec<u32>,
        is_second_coat: bool,
    ) -> Result<(), String> {
        let compiler = ExpressionCompiler::new();
        let mut expr = compiler.compile(&self.formula)?;

        let mut theta_val = 0.0_f64;
        expr.bind("theta", &mut theta_val);

        let base_index = (verts.len() / 3) as u32;
        let theta_step = (self.domain_end - self.domain_start) / self.sectors as f32;
        let normal_flip = if is_second_coat != self.reversed { -1.0 } else { 1.0 };

        // Generate vertices for cone (apex at top, base at bottom)
        // Apex vertex
        verts.extend_from_slice(&[0.0, 1.0, 0.0]);
        norms.extend_from_slice(&[0.0, normal_flip, 0.0]);

        // Base ring vertices
        for i in 0..=self.sectors {
            let theta = self.domain_start + theta_step * i as f32;
            theta_val = theta as f64;

            let r = expr.eval() as f32;
            let x = r * theta.cos();
            let z = r * theta.sin();

            verts.extend_from_slice(&[x, 0.0, z]);

            // Normal pointing outward and down for cone surface
            let nx = theta.cos() * normal_flip;
            let nz = theta.sin() * normal_flip;
            norms.extend_from_slice(&[nx, 0.5 * normal_flip, nz]);
        }

        // Generate indices for cone triangles
        for i in 0..self.sectors as u32 {
            let apex = base_index;
            let curr = base_index + 1 + i;
            let next = base_index + 1 + i + 1;

            if is_second_coat {
                indices.extend_from_slice(&[apex, next, curr]);
            } else {
                indices.extend_from_slice(&[apex, curr, next]);
            }
        }

        Ok(())
    }

    // Internal cone builder with colors
    fn build_cone_indexed_with_color_internal(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        colors: &mut Vec<f32>,
        indices: &mut Vec<u32>,
        is_second_coat: bool,
    ) -> Result<(), String> {
        let compiler = ExpressionCompiler::new();
        let mut expr = compiler.compile(&self.formula)?;

        let mut theta_val = 0.0_f64;
        expr.bind("theta", &mut theta_val);

        let base_index = (verts.len() / 3) as u32;
        let theta_step = (self.domain_end - self.domain_start) / self.sectors as f32;
        let normal_flip = if is_second_coat != self.reversed { -1.0 } else { 1.0 };
        let color = if is_second_coat { self.color_inner } else { self.color_outer };

        // Apex vertex
        verts.extend_from_slice(&[0.0, 1.0, 0.0]);
        norms.extend_from_slice(&[0.0, normal_flip, 0.0]);
        colors.extend_from_slice(&color);

        // Base ring vertices
        for i in 0..=self.sectors {
            let theta = self.domain_start + theta_step * i as f32;
            theta_val = theta as f64;

            let r = expr.eval() as f32;
            let x = r * theta.cos();
            let z = r * theta.sin();

            verts.extend_from_slice(&[x, 0.0, z]);

            let nx = theta.cos() * normal_flip;
            let nz = theta.sin() * normal_flip;
            norms.extend_from_slice(&[nx, 0.5 * normal_flip, nz]);
            colors.extend_from_slice(&color);
        }

        // Generate indices
        for i in 0..self.sectors as u32 {
            let apex = base_index;
            let curr = base_index + 1 + i;
            let next = base_index + 1 + i + 1;

            if is_second_coat {
                indices.extend_from_slice(&[apex, next, curr]);
            } else {
                indices.extend_from_slice(&[apex, curr, next]);
            }
        }

        Ok(())
    }

    // Internal cylinder builder
    fn build_cylinder_indexed_internal(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        indices: &mut Vec<u32>,
        is_second_coat: bool,
    ) -> Result<(), String> {
        let compiler = ExpressionCompiler::new();
        let mut expr = compiler.compile(&self.formula)?;

        let mut theta_val = 0.0_f64;
        expr.bind("theta", &mut theta_val);

        let base_index = (verts.len() / 3) as u32;
        let theta_step = (self.domain_end - self.domain_start) / self.sectors as f32;
        let y_step = 1.0 / self.slices as f32;
        let normal_flip = if is_second_coat != self.reversed { -1.0 } else { 1.0 };

        // Generate vertices for cylinder rings
        for j in 0..=self.slices {
            let y = j as f32 * y_step;

            for i in 0..=self.sectors {
                let theta = self.domain_start + theta_step * i as f32;
                theta_val = theta as f64;

                let r = expr.eval() as f32;
                let x = r * theta.cos();
                let z = r * theta.sin();

                verts.extend_from_slice(&[x, y, z]);

                // Normal pointing outward
                let nx = theta.cos() * normal_flip;
                let nz = theta.sin() * normal_flip;
                norms.extend_from_slice(&[nx, 0.0, nz]);
            }
        }

        // Generate indices for cylinder quads (as triangles)
        let ring_size = (self.sectors + 1) as u32;

        for j in 0..self.slices as u32 {
            for i in 0..self.sectors as u32 {
                let curr_ring = base_index + j * ring_size;
                let next_ring = base_index + (j + 1) * ring_size;

                let bl = curr_ring + i;
                let br = curr_ring + i + 1;
                let tl = next_ring + i;
                let tr = next_ring + i + 1;

                if is_second_coat {
                    // Reversed winding for inner surface
                    indices.extend_from_slice(&[bl, tl, br]);
                    indices.extend_from_slice(&[br, tl, tr]);
                } else {
                    indices.extend_from_slice(&[bl, br, tl]);
                    indices.extend_from_slice(&[br, tr, tl]);
                }
            }
        }

        Ok(())
    }

    // Internal cylinder builder with colors
    fn build_cylinder_indexed_with_color_internal(
        &self,
        verts: &mut Vec<f32>,
        norms: &mut Vec<f32>,
        colors: &mut Vec<f32>,
        indices: &mut Vec<u32>,
        is_second_coat: bool,
    ) -> Result<(), String> {
        let compiler = ExpressionCompiler::new();
        let mut expr = compiler.compile(&self.formula)?;

        let mut theta_val = 0.0_f64;
        expr.bind("theta", &mut theta_val);

        let base_index = (verts.len() / 3) as u32;
        let theta_step = (self.domain_end - self.domain_start) / self.sectors as f32;
        let y_step = 1.0 / self.slices as f32;
        let normal_flip = if is_second_coat != self.reversed { -1.0 } else { 1.0 };
        let color = if is_second_coat { self.color_inner } else { self.color_outer };

        // Generate vertices for cylinder rings
        for j in 0..=self.slices {
            let y = j as f32 * y_step;

            for i in 0..=self.sectors {
                let theta = self.domain_start + theta_step * i as f32;
                theta_val = theta as f64;

                let r = expr.eval() as f32;
                let x = r * theta.cos();
                let z = r * theta.sin();

                verts.extend_from_slice(&[x, y, z]);

                let nx = theta.cos() * normal_flip;
                let nz = theta.sin() * normal_flip;
                norms.extend_from_slice(&[nx, 0.0, nz]);
                colors.extend_from_slice(&color);
            }
        }

        // Generate indices
        let ring_size = (self.sectors + 1) as u32;

        for j in 0..self.slices as u32 {
            for i in 0..self.sectors as u32 {
                let curr_ring = base_index + j * ring_size;
                let next_ring = base_index + (j + 1) * ring_size;

                let bl = curr_ring + i;
                let br = curr_ring + i + 1;
                let tl = next_ring + i;
                let tr = next_ring + i + 1;

                if is_second_coat {
                    indices.extend_from_slice(&[bl, tl, br]);
                    indices.extend_from_slice(&[br, tl, tr]);
                } else {
                    indices.extend_from_slice(&[bl, br, tl]);
                    indices.extend_from_slice(&[br, tr, tl]);
                }
            }
        }

        Ok(())
    }
}

/// Convenience function to create a PolarBuilder
pub fn polar() -> PolarBuilder {
    PolarBuilder::new()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_cone() {
        let mut verts = Vec::new();
        let mut norms = Vec::new();
        let mut indices = Vec::new();

        PolarBuilder::new()
            .sectors(4)
            .build_cone_indexed(&mut verts, &mut norms, &mut indices)
            .unwrap();

        // Should have 1 apex + 5 base vertices (sectors + 1)
        assert_eq!(verts.len() / 3, 6);
        assert_eq!(norms.len() / 3, 6);
        // Should have 4 triangles
        assert_eq!(indices.len() / 3, 4);
    }

    #[test]
    fn test_build_cylinder() {
        let mut verts = Vec::new();
        let mut norms = Vec::new();
        let mut indices = Vec::new();

        PolarBuilder::new()
            .sectors(4)
            .slices(2)
            .build_cylinder_indexed(&mut verts, &mut norms, &mut indices)
            .unwrap();

        // Should have (sectors+1) * (slices+1) vertices = 5 * 3 = 15
        assert_eq!(verts.len() / 3, 15);
        // Should have sectors * slices * 2 triangles = 4 * 2 * 2 = 16
        assert_eq!(indices.len() / 3, 16);
    }

    #[test]
    fn test_formula() {
        let mut verts = Vec::new();
        let mut norms = Vec::new();
        let mut indices = Vec::new();

        PolarBuilder::new()
            .formula("1 + 0.5 * sin(4 * theta)")
            .sectors(8)
            .build_cone_indexed(&mut verts, &mut norms, &mut indices)
            .unwrap();

        assert!(!verts.is_empty());
    }

    #[test]
    fn test_double_coated() {
        let mut verts = Vec::new();
        let mut norms = Vec::new();
        let mut indices = Vec::new();

        let single_builder = PolarBuilder::new().sectors(4);
        single_builder.build_cone_indexed(&mut verts, &mut norms, &mut indices).unwrap();
        let single_indices = indices.len();

        verts.clear();
        norms.clear();
        indices.clear();

        let double_builder = PolarBuilder::new().sectors(4).double_coated(true);
        double_builder.build_cone_indexed(&mut verts, &mut norms, &mut indices).unwrap();

        // Double coated should have twice as many indices
        assert_eq!(indices.len(), single_indices * 2);
    }

    #[test]
    fn test_with_colors() {
        let mut verts = Vec::new();
        let mut norms = Vec::new();
        let mut colors = Vec::new();
        let mut indices = Vec::new();

        PolarBuilder::new()
            .sectors(4)
            .color_outer(1.0, 0.0, 0.0, 1.0)
            .build_cone_indexed_with_color(&mut verts, &mut norms, &mut colors, &mut indices)
            .unwrap();

        // Each vertex should have 4 color components
        assert_eq!(colors.len() / 4, verts.len() / 3);
    }
}
