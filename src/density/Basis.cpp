#include <stdio.h>
#include <math.h>

#include <algorithm>
#include <iostream>

#include "Basis.h"

#include "parameters.h"
#include "autogenerated.h"
#include "xcint_c_parameters.h"
#include "MemAllocator.h"


Basis::Basis()
{
    nullify();
}


Basis::~Basis()
{
    MemAllocator::deallocate(l_quantum_num);
    MemAllocator::deallocate(center_xyz);
    MemAllocator::deallocate(center_element);
    MemAllocator::deallocate(shell_center);
    MemAllocator::deallocate(shell_center_xyz);
    MemAllocator::deallocate(shell_extent_squared);
    MemAllocator::deallocate(cartesian_deg);
    MemAllocator::deallocate(shell_off);
    MemAllocator::deallocate(spherical_deg);
    MemAllocator::deallocate(ao_center);
    MemAllocator::deallocate(shell_num_primitives);
    MemAllocator::deallocate(geo_off);
    MemAllocator::deallocate(primitive_exp);
    MemAllocator::deallocate(contraction_coef);

    nullify();
}


void Basis::nullify()
{
    num_centers                  = -1;
    num_shells                   = -1;
    l_quantum_num                = NULL;
    center_xyz                   = NULL;
    center_element               = NULL;
    shell_center                 = NULL;
    shell_center_xyz             = NULL;
    shell_extent_squared         = NULL;
    cartesian_deg                = NULL;
    shell_off                    = NULL;
    spherical_deg                = NULL;
    is_spherical                 = false;
    num_ao                       = -1;
    num_ao_cartesian             = -1;
    num_ao_spherical             = -1;
    num_ao_slices                = -1;
    ao_center                    = NULL;
    shell_num_primitives     = NULL;
    geo_diff_order               = -1;
    geo_off                      = NULL;
    primitive_exp                = NULL;
    contraction_coef             = NULL;
    is_initialized               = false;
    is_synced                    = false;
}


void Basis::init(const int    in_basis_type,
                      const int    in_num_centers,
                      const double in_center_xyz[],
                      const int    in_center_element[],
                      const int    in_num_shells,
                      const int    in_shell_center[],
                      const int    in_l_quantum_num[],
                      const int    in_shell_num_primitives[],
                      const double in_primitive_exp[],
                      const double in_contraction_coef[])
{
    int i, l, deg, kc, ks;
    size_t block_size;

    if (is_initialized) return;

    num_centers = in_num_centers;

    switch (in_basis_type)
    {
        case XCINT_BASIS_SPHERICAL:
            is_spherical = true;
            break;
        case XCINT_BASIS_CARTESIAN:
            is_spherical = false;
            fprintf(stderr, "ERROR: XCINT_BASIS_CARTESIAN not tested.\n");
            exit(-1);
            break;
        default:
            fprintf(stderr, "ERROR: basis_type not recognized.\n");
            exit(-1);
            break;
    }

    num_shells = in_num_shells;

    block_size = 3*num_centers*sizeof(double);
    center_xyz = (double*) MemAllocator::allocate(block_size);
    std::copy(&in_center_xyz[0], &in_center_xyz[3*num_centers], &center_xyz[0]);

    block_size = num_centers*sizeof(int);
    center_element = (int*) MemAllocator::allocate(block_size);
    std::copy(&in_center_element[0], &in_center_element[num_centers], &center_element[0]);

    block_size = num_shells*sizeof(int);
    shell_center = (int*) MemAllocator::allocate(block_size);
    std::copy(&in_shell_center[0], &in_shell_center[num_shells], &shell_center[0]);

    block_size = 3*num_shells*sizeof(double);
    shell_center_xyz = (double*) MemAllocator::allocate(block_size);

    for (int ishell = 0; ishell < num_shells; ishell++)
    {
        for (int ixyz = 0; ixyz < 3; ixyz++)
        {
            shell_center_xyz[3*ishell + ixyz] = in_center_xyz[3*(in_shell_center[ishell]-1) + ixyz];
        }
    }

    block_size = num_shells*sizeof(int);
    l_quantum_num = (int*) MemAllocator::allocate(block_size);
    std::copy(&in_l_quantum_num[0], &in_l_quantum_num[num_shells], &l_quantum_num[0]);

    for (int ishell = 0; ishell < num_shells; ishell++)
    {
        if (l_quantum_num[ishell] > MAX_L_VALUE)
        {
            fprintf(stderr, "ERROR: increase MAX_L_VALUE.\n");
            exit(-1);
        }
    }

    block_size = num_shells*sizeof(int);
    shell_num_primitives = (int*) MemAllocator::allocate(block_size);
    std::copy(&in_shell_num_primitives[0], &in_shell_num_primitives[num_shells], &shell_num_primitives[0]);

    int n = 0;
    for (int ishell = 0; ishell < num_shells; ishell++)
    {
        n += shell_num_primitives[ishell];
    }

    block_size = n*sizeof(double);
    primitive_exp = (double*) MemAllocator::allocate(block_size);
    contraction_coef = (double*) MemAllocator::allocate(block_size);
    std::copy(&in_primitive_exp[0], &in_primitive_exp[n], &primitive_exp[0]);
    std::copy(&in_contraction_coef[0], &in_contraction_coef[n], &contraction_coef[0]);

    // get approximate spacial shell extent
    double SHELL_SCREENING_THRESHOLD = 2.0e-12;
    double e, c, r, r_temp;
    // threshold and factors match Dalton implementation, see also pink book
    double f[10] = {1.0, 1.3333, 1.6, 1.83, 2.03, 2.22, 2.39, 2.55, 2.70, 2.84};
    block_size = num_shells*sizeof(double);
    shell_extent_squared = (double*) MemAllocator::allocate(block_size);
    n = 0;
    for (int ishell = 0; ishell < num_shells; ishell++)
    {
        r = 0.0;
        for (int j = 0; j < shell_num_primitives[ishell]; j++)
        {
            e = primitive_exp[n];
            c = contraction_coef[n];
            n++;
            r_temp = (log(fabs(c)) - log(SHELL_SCREENING_THRESHOLD))/e;
            if (r_temp > r) r = r_temp;
        }
        if (l_quantum_num[ishell] < 10)
        {
            r = pow(r, 0.5)*f[l_quantum_num[ishell]];
        }
        else
        {
            r = 1.0e10;
        }
        shell_extent_squared[ishell] = r*r;
    }

    block_size = num_shells*sizeof(int);
    cartesian_deg = (int*) MemAllocator::allocate(block_size);
    shell_off = (int*) MemAllocator::allocate(block_size);
    spherical_deg = (int*) MemAllocator::allocate(block_size);

    num_ao_cartesian = 0;
    num_ao_spherical = 0;
    for (int ishell = 0; ishell < num_shells; ishell++)
    {
        l = l_quantum_num[ishell];
        kc = (l+1)*(l+2)/2;
        ks = 2*l + 1;
        cartesian_deg[ishell] = kc;
        spherical_deg[ishell] = ks;

        if (is_spherical)
        {
            shell_off[ishell] = num_ao_spherical;
        }
        else
        {
            shell_off[ishell] = num_ao_cartesian;
        }

        num_ao_cartesian += kc;
        num_ao_spherical += ks;
    }

    if (is_spherical)
    {
        num_ao = num_ao_spherical;
    }
    else
    {
        fprintf(stderr, "ERROR: XCint probably broken for cart basis, needs testing.\n");
        exit(-1);
        num_ao = num_ao_cartesian;
    }

    block_size = num_ao*sizeof(int);
    ao_center = (int*) MemAllocator::allocate(block_size);
    i = 0;
    for (int ishell = 0; ishell < num_shells; ishell++)
    {
       if (is_spherical)
       {
           deg = spherical_deg[ishell];
       }
       else
       {
           deg = cartesian_deg[ishell];
       }
       for (int j = i; j < (i + deg); j++)
       {
           ao_center[j] = in_shell_center[ishell] - 1;
       }

       i += deg;
    }

    set_geo_off(MAX_GEO_DIFF_ORDER); // FIXME

    is_initialized = true;
}


void Basis::set_geo_off(const int g)
{
    int i, j, k, m, id;
    int array_length = (int)pow(g + 1, 3);

    geo_diff_order = g;

    size_t block_size = array_length*sizeof(int);

    if (geo_off != NULL) MemAllocator::deallocate(geo_off);
    geo_off = (int*) MemAllocator::allocate(block_size);

    m = 0;
    for (int l = 0; l <= g; l++)
    {
        for (int a = 1; a <= (l + 1); a++)
        {
            for (int b = 1; b <= a; b++)
            {
                i = l + 1 - a;
                j = a - b;
                k = b - 1;

                id  = (g + 1)*(g + 1)*k;
                id += (g + 1)*j;
                id += i;
                geo_off[id] = m*num_ao;

                m++;
            }
        }
    }
    num_ao_slices = m;
}


int Basis::get_geo_off(const int i, const int j, const int k) const
{
    int id;
    // FIXME add guard against going past the array
    id  = (geo_diff_order + 1)*(geo_diff_order + 1)*k;
    id += (geo_diff_order + 1)*j;
    id += i;
    return geo_off[id];
}


int Basis::get_num_centers() const
{
    return num_centers;
}


int Basis::get_num_ao_slices() const
{
    return num_ao_slices;
}


int Basis::get_num_ao() const
{
    return num_ao;
}


int Basis::get_num_ao_cartesian() const
{
    return num_ao_cartesian;
}


int Basis::get_ao_center(const int i) const
{
    return ao_center[i];
}
