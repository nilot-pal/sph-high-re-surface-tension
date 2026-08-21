/**
 * @file 	fluid_dynamics_inner.hpp
 * @brief 	Here, we define the algorithm classes for fluid dynamics within the body.
 * @details 	We consider here weakly compressible fluids. The algorithms may be
 * 			different for free surface flow and the one without free surface.
 * @author	Chi Zhang and Xiangyu Hu
 */
#pragma once

#include "fluid_dynamics_inner.h"

namespace SPH
{
//=====================================================================================================//
namespace fluid_dynamics
{
//=================================================================================================//
void DensitySummationInner::
    interaction(size_t index_i, Real dt)
{
    Real sigma = W0_;
    const Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
        sigma += inner_neighborhood.W_ij_[n];

    rho_sum_[index_i] = sigma * rho0_ * inv_sigma0_;
}
//=================================================================================================//
void DensitySummationInnerAdaptive::
    interaction(size_t index_i, Real dt)
{
    Real sigma_i = mass_[index_i] * kernel_.W0(h_ratio_[index_i], ZeroVecd);
    const Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
        sigma_i += inner_neighborhood.W_ij_[n] * mass_[inner_neighborhood.j_[n]];

    rho_sum_[index_i] = sigma_i * rho0_ * inv_sigma0_ / mass_[index_i] /
                        sph_adaptation_.NumberDensityScaleFactor(h_ratio_[index_i]);
}
//=================================================================================================//
void ViscousAccelerationInner::
    interaction(size_t index_i, Real dt)
{
    Vecd acceleration = Vecd::Zero();
    Vecd vel_derivative = Vecd::Zero();
    const Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    {
        size_t index_j = inner_neighborhood.j_[n];

        // viscous force
        vel_derivative = (vel_[index_i] - vel_[index_j]) / (inner_neighborhood.r_ij_[n] + 0.01 * smoothing_length_);
        acceleration += 2.0 * mu_ * vel_derivative * inner_neighborhood.dW_ijV_j_[n];
    }

    acc_prior_[index_i] += acceleration / rho_[index_i];
}
//=================================================================================================//
void AngularConservativeViscousAccelerationInner::
    interaction(size_t index_i, Real dt)
{
    Vecd acceleration = Vecd::Zero();
    Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    {
        size_t index_j = inner_neighborhood.j_[n];
        Vecd &e_ij = inner_neighborhood.e_ij_[n];
        Real r_ij = inner_neighborhood.r_ij_[n];

        /** The following viscous force is given in Monaghan 2005 (Rep. Prog. Phys.), it seems that
         * this formulation is more accurate than the previous one for Taylor-Green-Vortex flow. */
        Real v_r_ij = (vel_[index_i] - vel_[index_j]).dot(r_ij * e_ij);
        Real eta_ij = 8.0 * mu_ * v_r_ij / (r_ij * r_ij + 0.01 * smoothing_length_);
        acceleration += eta_ij * inner_neighborhood.dW_ijV_j_[n] * e_ij;
    }

    acc_prior_[index_i] += acceleration / rho_[index_i];
}
//=================================================================================================//
void VorticityInner::interaction(size_t index_i, Real dt)
{
    AngularVecd vorticity = ZeroData<AngularVecd>::value;
    const Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    {
        size_t index_j = inner_neighborhood.j_[n];

        Vecd vel_diff = vel_[index_i] - vel_[index_j];
        vorticity += getCrossProduct(vel_diff, inner_neighborhood.e_ij_[n]) * inner_neighborhood.dW_ijV_j_[n];
    }

    vorticity_[index_i] = vorticity;
}
//=================================================================================================//
void Oldroyd_BIntegration1stHalf::
    interaction(size_t index_i, Real dt)
{
    Integration1stHalfDissipativeRiemann::interaction(index_i, dt);

    Vecd acceleration = Vecd::Zero();
    Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    {
        size_t index_j = inner_neighborhood.j_[n];
        Vecd nablaW_ijV_j = inner_neighborhood.dW_ijV_j_[n] * inner_neighborhood.e_ij_[n];

        // elastic force
        acceleration += (tau_[index_i] + tau_[index_j]) * nablaW_ijV_j;
    }

    acc_[index_i] += acceleration / rho_[index_i];
}
//=================================================================================================//
void Oldroyd_BIntegration2ndHalf::
    interaction(size_t index_i, Real dt)
{
    Integration2ndHalfDissipativeRiemann::interaction(index_i, dt);

    Matd tau_i = tau_[index_i];
    Matd stress_rate = Matd::Zero();
    Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    {
        size_t index_j = inner_neighborhood.j_[n];
        Vecd nablaW_ijV_j = inner_neighborhood.dW_ijV_j_[n] * inner_neighborhood.e_ij_[n];

        Matd velocity_gradient = -(vel_[index_i] - vel_[index_j]) * nablaW_ijV_j.transpose();
        stress_rate += velocity_gradient.transpose() * tau_i + tau_i * velocity_gradient - tau_i / lambda_ +
                       (velocity_gradient.transpose() + velocity_gradient) * mu_p_ / lambda_;
    }

    dtau_dt_[index_i] = stress_rate;
}
//=================================================================================================//
template <class RiemannSolverType>
BaseIntegration1stHalf<RiemannSolverType>::BaseIntegration1stHalf(BaseInnerRelation &inner_relation)
    : BaseIntegration(inner_relation), riemann_solver_(fluid_, fluid_), mass_(particles_->mass_)
{
    /**
     *	register sortable particle data
     */
    particles_->registerSortableVariable<Vecd>("Position");
    particles_->registerSortableVariable<Vecd>("Velocity");
    particles_->registerSortableVariable<Real>("MassiveMeasure");
    particles_->registerSortableVariable<Real>("Density");
    particles_->registerSortableVariable<Real>("Pressure");
    particles_->registerSortableVariable<Real>("VolumetricMeasure");
    //----------------------------------------------------------------------
    //		add restart output particle data
    //----------------------------------------------------------------------
    particles_->addVariableToRestart<Real>("Pressure");
}
//=================================================================================================//
template <class RiemannSolverType>
void BaseIntegration1stHalf<RiemannSolverType>::initialization(size_t index_i, Real dt)
{
    rho_[index_i] += drho_dt_[index_i] * dt * 0.5;
    p_[index_i] = fluid_.getPressure(rho_[index_i]);
    pos_[index_i] += vel_[index_i] * dt * 0.5;
}
//=================================================================================================//
template <class RiemannSolverType>
void BaseIntegration1stHalf<RiemannSolverType>::update(size_t index_i, Real dt)
{
    vel_[index_i] += (acc_prior_[index_i] + acc_[index_i]) * dt;
}
//=================================================================================================//
template <class RiemannSolverType>
Vecd BaseIntegration1stHalf<RiemannSolverType>::computeNonConservativeAcceleration(size_t index_i)
{
    Vecd acceleration = acc_prior_[index_i] * rho_[index_i];
    const Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    {
        size_t index_j = inner_neighborhood.j_[n];
        Real dW_ijV_j = inner_neighborhood.dW_ijV_j_[n];
        const Vecd &e_ij = inner_neighborhood.e_ij_[n];

        acceleration += (p_[index_i] - p_[index_j]) * dW_ijV_j * e_ij;
    }
    return acceleration / rho_[index_i];
}
//=================================================================================================//
template <class RiemannSolverType>
void BaseIntegration1stHalf<RiemannSolverType>::
    interaction(size_t index_i, Real dt)
{
    Vecd acceleration = Vecd::Zero();
    Real rho_dissipation(0);
    const Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    {
        size_t index_j = inner_neighborhood.j_[n];
        //Real r_ij = inner_neighborhood.r_ij_[n];
        //Real dW_ij = inner_neighborhood.dW_ij_[n];
        Real dW_ijV_j = inner_neighborhood.dW_ijV_j_[n];
        const Vecd &e_ij = inner_neighborhood.e_ij_[n];
        Real q_i = 1.0;
        Real q_j = 1.0;
        Real u_jump = -(vel_[index_i] - vel_[index_j]).dot(e_ij); //e_ij (code) = -e_ij (literature)
        Real c_i = fluid_.ReferenceSoundSpeed();
        Real c_j = fluid_.ReferenceSoundSpeed();
        Real p_star = (rho_[index_i]*c_i*p_[index_j] + rho_[index_j]*c_j*p_[index_i])/(rho_[index_i]*c_i + rho_[index_j]*c_j);
        p_star = SMAX(Real(0), p_star + 0.5*riemann_solver_.DissipativePJump(u_jump));
        //Real p_star =  riemann_solver_.DissipativePJump(u_jump);
        if (p_star > p_[index_i]) {
        	Real gamma = 7.0;
        	q_i = sqrt(1 + (gamma+1)*(p_star/p_[index_i] -1)/(2*gamma));
        }
        if (p_star > p_[index_j]) {
        	Real gamma = 7.0;
        	q_j = sqrt(1 + (gamma+1)*(p_star/p_[index_j] -1)/(2*gamma));
        }
        Real u_L = -vel_[index_i].dot(e_ij);
		Real u_R = -vel_[index_j].dot(e_ij);
		StdVec<Real>UHllc = riemann_solver_.UHllc(rho_[index_i], rho_[index_j], p_[index_i], p_[index_j], vel_[index_i] , vel_[index_j], u_L, u_R, c_i, c_j, q_i, q_j);
        Vecd vel_Hllc = Vecd::Zero();
        vel_Hllc[0] = UHllc[1];
        vel_Hllc[1] = UHllc[2];
        Real u_hllc = - vel_Hllc.dot(e_ij);
        // Real gamma_new = 1.4;
        // Real rho_ref = fluid_.ReferenceDensity();
        // Real pHllc = rho_ref*pow(1,2)/gamma_new*(pow(UHllc[0]/rho_ref , gamma_new) -1);
        // acceleration -= 2 * pHllc * dW_ijV_j * e_ij; 
        acceleration -= 2*fluid_.getPressure(UHllc[0]) * dW_ijV_j * e_ij;
        Real h_custom = 1.3*0.025;
        //rho_dissipation += 2*UHllc[1]* dW_ijV_j /h_custom* (rho_[index_j] / mass_[index_j]);
        rho_dissipation += 2*u_hllc* dW_ijV_j/h_custom; 
    }
    acc_[index_i] += acceleration / rho_[index_i];
    drho_dt_[index_i] = rho_dissipation * rho_[index_i];
}
//=================================================================================================//
template <class RiemannSolverType>
BaseIntegration2ndHalf<RiemannSolverType>::BaseIntegration2ndHalf(BaseInnerRelation &inner_relation)
    : BaseIntegration(inner_relation), riemann_solver_(fluid_, fluid_),
      Vol_(particles_->Vol_), mass_(particles_->mass_) {}
//=================================================================================================//
template <class RiemannSolverType>
void BaseIntegration2ndHalf<RiemannSolverType>::initialization(size_t index_i, Real dt)
{
    pos_[index_i] += vel_[index_i] * dt * 0.5;
}
//=================================================================================================//
template <class RiemannSolverType>
void BaseIntegration2ndHalf<RiemannSolverType>::update(size_t index_i, Real dt)
{
    rho_[index_i] += drho_dt_[index_i] * dt * 0.5;
    Vol_[index_i] = mass_[index_i] / rho_[index_i];
}
//=================================================================================================//
template <class RiemannSolverType>
void BaseIntegration2ndHalf<RiemannSolverType>::
    interaction(size_t index_i, Real dt)
{
    Real density_change_rate(0);
    //Vecd p_dissipation = Vecd::Zero();
    const Neighborhood &inner_neighborhood = inner_configuration_[index_i];
    //for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    //{
      //  size_t index_j = inner_neighborhood.j_[n];
        //const Vecd &e_ij = inner_neighborhood.e_ij_[n];
    	//Real dW_ijV_j = inner_neighborhood.dW_ijV_j_[n];

    	//Real u_jump = (vel_[index_i] - vel_[index_j]).dot(e_ij);
       	//density_change_rate += u_jump * dW_ijV_j;
       	//p_dissipation += riemann_solver_.DissipativePJump(u_jump) * dW_ijV_j * e_ij;
   	//}
   	for (size_t n = 0; n != inner_neighborhood.current_size_; ++n)
    	{
        	//size_t index_j = inner_neighborhood.j_[n];
        	//Real r_ij = inner_neighborhood.r_ij_[n];
        	const Vecd &e_ij = inner_neighborhood.e_ij_[n];
            //Real dW_ij = inner_neighborhood.dW_ij_[n];
        	Real dW_ijV_j = inner_neighborhood.dW_ijV_j_[n];
			Real u_L = -vel_[index_i].dot(e_ij);
			Real h_custom = 1.3*0.025;
        	//density_change_rate -= 2*u_L* dW_ijV_j /h_custom* (rho_[index_j] / mass_[index_j]);
            density_change_rate -= 2*u_L* dW_ijV_j /h_custom;
        
    	}
    drho_dt_[index_i] += density_change_rate * rho_[index_i];
  	//drho_dt_[index_i] += density_change_rate * rho_[index_i];
   	//acc_[index_i] = p_dissipation / rho_[index_i];
}
//=================================================================================================//
} // namespace fluid_dynamics
  //=================================================================================================//
} // namespace SPH
  //=================================================================================================//
