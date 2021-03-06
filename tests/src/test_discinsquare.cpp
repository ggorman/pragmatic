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

#include "Mesh.h"
#ifdef HAVE_LIBMESHB
#include "GMFTools.h"
#endif
#ifdef HAVE_VTK
#include "VTKTools.h"
#endif
#include "MetricField.h"
#include "Refine.h"
#include "ticker.h"
#include "cpragmatic.h"
#include "Coarsen.h"
#include "Swapping.h"

#include <mpi.h>

int main(int argc, char **argv)
{
    int rank=0;
    int required_thread_support=MPI_THREAD_SINGLE;
    int provided_thread_support;
    MPI_Init_thread(&argc, &argv, required_thread_support, &provided_thread_support);
    assert(required_thread_support==provided_thread_support);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    bool verbose = false;
    if(argc>1) {
        verbose = std::string(argv[1])=="-v";
    }

#ifdef HAVE_LIBMESHB
    Mesh<double> *mesh=GMFTools<double>::import_gmf_mesh("../data/disc");
    pragmatic_init_light((void*)mesh);

    int * boundary = mesh->get_boundaryTags();
    mesh->set_internal_boundaries();

    MetricField<double,2> metric_field(*mesh);

    size_t NNodes = mesh->get_number_nodes();
    
    double m[3] = {0.};
    double hmax = 1.;
    double hmin = 0.05;
    for(size_t i=0; i<NNodes; i++) {
        double x = mesh->get_coords(i)[0];
        double h = hmax*fabs(1-exp(-0.03*fabs(x*x*x))) + hmin;
        double lmax = 1/(hmax*hmax);
        m[0] = 1/(h*h);
        m[2] = lmax;
        metric_field.set_metric(m, i);
    }
    metric_field.update_mesh();

    GMFTools<double>::export_gmf_mesh("../data/test_discinsquare-initial", mesh);

    pragmatic_adapt(0, 1);

    if(verbose)
        mesh->verify();
    mesh->defragment();

    GMFTools<double>::export_gmf_mesh("../data/test_discinsquare", mesh);
#ifdef HAVE_VTK
    VTKTools<double>::export_vtu("../data/test_discinsquare", mesh);
#else
    std::cerr<<"Warning: Pragmatic was configured without VTK support"<<std::endl;
#endif

    long double perimeter = mesh->calculate_perimeter(11, 12);
    long double area = mesh->calculate_area(12);

    long double ideal_perimeter(18.85), ideal_area(28.27); // the internal boundary is counted twice
    std::cout<<"Expecting perimeter ~= 18.85: ";
    if(std::abs(perimeter-ideal_perimeter)/std::max(perimeter, ideal_perimeter)<0.05)
        std::cout<<"pass"<<std::endl;
    else
        std::cout<<"fail: "<<perimeter<<std::endl;

    std::cout<<"Expecting area == 28.27: ";
    if(std::abs(area-ideal_area)/std::max(area, ideal_area)<0.05)
        std::cout<<"pass"<<std::endl;
    else
        std::cout<<"fail: "<<area<<std::endl;

    delete mesh;
#else
    std::cerr<<"Pragmatic was configured without libmeshb"<<std::endl;
#endif

    MPI_Finalize();

    return 0;
}
