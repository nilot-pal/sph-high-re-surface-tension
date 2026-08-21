#include "riemann_solver.h"
#include "base_material.h"

namespace SPH
{
//=================================================================================================//
Real NoRiemannSolver::DissipativePJump(const Real &u_jump)
{
    return 0.0;
}
//=================================================================================================//
Real NoRiemannSolver::DissipativeUJump(const Real &p_jump)
{
    return 0.0;
}
//=================================================================================================//
Real NoRiemannSolver::AverageP(const Real &p_i, const Real &p_j)
{
    return (p_i * rho0c0_j_ + p_j * rho0c0_i_) * inv_rho0c0_sum_;
}
//=================================================================================================//
Vecd NoRiemannSolver::AverageV(const Vecd &vel_i, const Vecd &vel_j)
{
    return (vel_i * rho0c0_i_ + vel_j * rho0c0_j_) * inv_rho0c0_sum_;
}

StdVec<Real>NoRiemannSolver::UHllc(Real &rho_L, Real &rho_R, Real &p_L, Real &p_R, Vecd &u_i, Vecd &u_j, Real &u_L, Real &u_R, Real &c_L, Real &c_R, Real &q_L, Real &q_R) {
	StdVec<Real>UHLLC = {0,0,0};
	return UHLLC;
}
//=================================================================================================//
Real AcousticRiemannSolver::DissipativePJump(const Real &u_jump)
{
    // Ref: Smoothed particle hydrodynamics: Methodology development and recent achievement
    return rho0c0_geo_ave_ * u_jump *SMIN(Real(3) * SMAX(u_jump * inv_c_ave_, Real(0)), Real(1));
    //return rho0c0_geo_ave_ * u_jump;
}
//=================================================================================================//
Real AcousticRiemannSolver::DissipativeUJump(const Real &p_jump)
{
    return p_jump * inv_rho0c0_ave_;
}
//=================================================================================================//
Real DissipativeRiemannSolver::DissipativePJump(const Real &u_jump)
{
    return rho0c0_geo_ave_ * u_jump;
}
//=================================================================================================//
/** HLLC Riemann solver **/

StdVec<Real>AcousticRiemannSolver::UHllc(Real &rho_L, Real &rho_R, Real &p_L, Real &p_R, Vecd &u_i, Vecd &u_j, Real &u_L, Real &u_R, Real &c_L, Real &c_R, Real &q_L, Real &q_R)
{
	StdVec<Real>UHLLC = {};
	Real rho, u, v;
	Real S_L = u_L  - c_L*q_L;	
	Real S_R = u_R + c_R*q_R;
	Real S_star = (p_R-p_L + rho_L*u_L*(S_L-u_L) - rho_R*u_R*(S_R-u_R))/(rho_L*(S_L-u_L) - rho_R*(S_R-u_R));
	if (S_L >= 0) {	
		rho = rho_L;
		u = u_i[0];
		v = u_i[1];
	}
	else if (S_L <= 0 && S_star >= 0) {
		rho = rho_L*(S_L-u_L)/(S_L-S_star);
		u = S_star;
		v = u_i[1];
	}
	else if (S_star <= 0 && S_R >= 0) {
		rho = rho_R*(S_R-u_R)/(S_R-S_star);
		u = S_star;
		v = u_j[1];
	}
	else {
		rho = rho_R;
		u = u_j[0];
		v = u_j[1];	
	}
	UHLLC.insert(UHLLC.end(),{rho,u,v});
	return UHLLC;
}
//=================================================================================================//
} // namespace SPH
