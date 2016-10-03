/*  Copyright (C) 2010 Imperial College London and others.
 *
 *  Please see the AUTHORS file in the main source directory for a
 *  full list of copyright holders.
 *
 *  Gerard Gorman
 *  Applied Modelling and Computation Group
 *  Department of Earth Science and Engineering
 *  Imperial College London
 *
 *  g.gorman@imperial.ac.uk
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials provided
 *  with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */

#include <iostream>
#include <vector>
#include <errno.h>
#include <cfloat>

#ifdef HAVE_OPENMP
#include <omp.h>
#endif

#include "Mesh.h"
#ifdef HAVE_VTK
#include "VTKTools.h"
#endif
#include "MetricField.h"
#include "Smooth.h"
#include "ticker.h"

#ifdef HAVE_MPI
#include <mpi.h>
#endif

void test_block(double qmean, double qmin, long double area, long double volume)
{
    long double ideal_area(6), ideal_volume(1);

    std::cout<<"Checking area == 6: ";
    if(std::abs(area-ideal_area)/std::max(area, ideal_area)<DBL_EPSILON)
        std::cout<<"pass"<<std::endl;
    else
        std::cout<<"fail (area="<<area<<" diff="<<std::abs(area-ideal_area)<<")"<<std::endl;

    std::cout<<"Checking volume == 1: ";
    if(std::abs(volume-ideal_volume)/std::max(volume, ideal_volume)<DBL_EPSILON)
        std::cout<<"pass"<<std::endl;
    else
        std::cout<<"fail (volume="<<volume<<")"<<std::endl;
}

int main(int argc, char **argv)
{
    int rank=0;
#ifdef HAVE_MPI
    int required_thread_support=MPI_THREAD_SINGLE;
    int provided_thread_support;
    MPI_Init_thread(&argc, &argv, required_thread_support, &provided_thread_support);
    assert(required_thread_support==provided_thread_support);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

    const double target_quality_mean = 0.3;
    const double target_quality_min = 0.01;

#ifdef HAVE_VTK
    Mesh<double> *mesh=VTKTools<double>::import_vtu("../data/box20x20x20.vtu");
    mesh->create_boundary();

    MetricField<double,3> metric_field(*mesh);

    size_t NNodes = mesh->get_number_nodes();

    double h1 = 1.0/20;
    double h0 = 10.0/20;
    for(size_t i=0; i<NNodes; i++) {
        // Want x,y,z ranging from -1, 1
        double x = 2*mesh->get_coords(i)[0] - 1;
        double y = 2*mesh->get_coords(i)[1] - 1;
        double z = 2*mesh->get_coords(i)[2] - 1;
        double d = std::min(1-std::abs(x), std::min(1-std::abs(y), 1-std::abs(z)));

        double hx = h0 - (h1-h0)*(d-1);
        double m[] = {1.0/pow(hx, 2), 0,              0,
                      1.0/pow(hx, 2), 0,
                      1.0/pow(hx, 2)
        };

        metric_field.set_metric(m, i);
    }
    metric_field.update_mesh();

    double qmean = mesh->get_qmean();
    double qmin = mesh->get_qmin();

    long double area = mesh->calculate_area();
    long double volume = mesh->calculate_volume();

    if(rank==0) {
        test_block(qmean, qmin, area, volume);
    }

    Smooth<double, 3> smooth(*mesh);
    double tic, toc;
    std::string vtu_filename;
    // =================================
#if 0
    tic = get_wtime();
    smooth.condition_number(200);
    toc = get_wtime();

    qmean = mesh->get_qmean();
    qmin = mesh->get_qmin();

    area = mesh->calculate_area();
    volume = mesh->calculate_volume();

    if(rank==0) {
        std::cout << "Condtion number time  " << toc-tic << std::endl;
        test_block(qmean, qmin, area, volume);
    }

    std::string vtu_filename = std::string("../data/test_condition_number_3d");
    VTKTools<double>::export_vtu(vtu_filename.c_str(), mesh);
#endif
    // =================================

    tic = get_wtime();
    smooth.laplacian(100);
    toc = get_wtime();

    qmean = mesh->get_qmean();
    qmin = mesh->get_qmin();

    area = mesh->calculate_area();
    volume = mesh->calculate_volume();

    if(rank==0) {
        std::cout<<"Laplacian smooth time  "<<toc-tic<<std::endl;
        test_block(qmean, qmin, area, volume);
    }

    vtu_filename = std::string("../data/test_smooth_laplacian_3d");
    VTKTools<double>::export_vtu(vtu_filename.c_str(), mesh);

    // =================================
    tic = get_wtime();
    smooth.smart_laplacian(200);
    toc = get_wtime();

    qmean = mesh->get_qmean();
    qmin = mesh->get_qmin();

    area = mesh->calculate_area();
    volume = mesh->calculate_volume();

    if(rank==0) {
        std::cout<<"Smart Laplacian smooth time  "<<toc-tic<<std::endl;
        test_block(qmean, qmin, area, volume);
    }

    vtu_filename = std::string("../data/test_smooth_smart_laplacian_3d");
    VTKTools<double>::export_vtu(vtu_filename.c_str(), mesh);

    // =================================
    tic = get_wtime();
    smooth.optimisation_linf(400);
    toc = get_wtime();

    qmean = mesh->get_qmean();
    qmin = mesh->get_qmin();

    area = mesh->calculate_area();
    volume = mesh->calculate_volume();

    if(rank==0) {
        std::cout<<"Linf optimisation smooth time  "<<toc-tic<<std::endl;
        test_block(qmean, qmin, area, volume);
    }

    vtu_filename = std::string("../data/test_smooth_optimisation_linf_3d");
    VTKTools<double>::export_vtu(vtu_filename.c_str(), mesh);
#else
    std::cerr<<"Pragmatic was configured without VTK"<<std::endl;
#endif

#ifdef HAVE_MPI
    MPI_Finalize();
#endif

    return 0;
}
