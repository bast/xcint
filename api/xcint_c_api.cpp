
#include "XCint.h"
#include "xcint_c_api.h"

XCint xc;

extern "C"
{
    int xcint_set_functional(const char *line, double &hfx, double &mu, double &beta)
    {
        xc.set_functional(line, hfx, mu, beta);
        return 0;
    }


    void xcint_set_basis(const int    basis_type,
                         const int    num_centers,
                         const double center_xyz[],
                         const int    center_element[],
                         const int    num_shells,
                         const int    shell_center[],
                         const int    l_quantum_num[],
                         const int    shell_num_primitives[],
                         const double primitive_exp[],
                         const double contraction_coef[])
    {
        xc.set_basis(basis_type,
                     num_centers,
                     center_xyz,
                     center_element,
                     num_shells,
                     shell_center,
                     l_quantum_num,
                     shell_num_primitives,
                     primitive_exp,
                     contraction_coef);
    }


    void xcint_integrate(const int    mode,
                         const int    num_points,
                         const double grid_pw[],
                         const int    num_pert,
                         const int    pert[],
                         const int    comp[],
                         const int    num_dmat,
                         const int    dmat_to_pert[],
                         const int    dmat_to_comp[],
                               double dmat[],
                         const int    get_xc_energy,
                               double &xc_energy,
                         const int    get_xc_mat,
                               double xc_mat[],
                               double &num_electrons)
    {
        xc.integrate(mode,
                     num_points,
                     grid_pw,
                     num_pert,
                     pert,
                     comp,
                     num_dmat,
                     dmat_to_pert,
                     dmat_to_comp,
                     dmat,
                     get_xc_energy,
                     xc_energy,
                     get_xc_mat,
                     xc_mat,
                     num_electrons);
    }
}
