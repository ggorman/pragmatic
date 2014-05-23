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

#include <cassert>

#include "Mesh.h"
#include "MetricField.h"
#include "Coarsen.h"
#include "Refine.h"
#include "Swapping.h"
#include "Smooth.h"

#include "VTKTools.h"

static void *_pragmatic_mesh=NULL;
static void *_pragmatic_metric_field=NULL;

extern "C" {
  void pragmatic_dump(const char *filename){
    VTKTools<double>::export_vtu(filename, (Mesh<double>*)_pragmatic_mesh);
  }

  void pragmatic_dump_debug(){
    pragmatic_dump("dump\0");
  }

  /** Initialise pragmatic with mesh to be adapted. pragmatic_finalize must
      be called before this can be called again, i.e. cannot adapt
      multiple meshes at the same time.

      @param [in] NNodes Number of nodes
      @param [in] NElements Number of elements
      @param [in] enlist Element-node list
      @param [in] x x coordinate array
      @param [in] y y coordinate array
   */
  void pragmatic_2d_init(const int *NNodes, const int *NElements, const int *enlist, const double *x, const double *y){
    if(_pragmatic_mesh!=NULL){
      throw new std::string("PRAgMaTIc: only one mesh can be adapted at a time");
    }

    Mesh<double> *mesh = new Mesh<double>(*NNodes, *NElements, enlist, x, y);

    _pragmatic_mesh = mesh;
  }

  /** Initialise pragmatic with mesh to be adapted. pragmatic_finalize must
      be called before this can be called again, i.e. cannot adapt
      multiple meshes at the same time.

      @param [in] NNodes Number of nodes
      @param [in] NElements Number of elements
      @param [in] enlist Element-node list
      @param [in] x x coordinate array
      @param [in] y y coordinate array
      @param [in] z z coordinate array
   */
  void pragmatic_3d_init(const int *NNodes, const int *NElements, const int *enlist, const double *x, const double *y, const double *z){
    assert(_pragmatic_mesh==NULL);
    assert(_pragmatic_metric_field==NULL);

    Mesh<double> *mesh = new Mesh<double>(*NNodes, *NElements, enlist, x, y, z);

    _pragmatic_mesh = mesh;
  }

  /** Initialise pragmatic with name of VTK file to be adapted.
  */
  void pragmatic_vtk_init(const char *filename){
    assert(_pragmatic_mesh==NULL);
    assert(_pragmatic_metric_field==NULL);

    Mesh<double> *mesh=VTKTools<double>::import_vtu(filename);
    mesh->create_boundary();

    _pragmatic_mesh = mesh;
  }

  /** Add field which should be adapted to.

      @param [in] psi Node centred field variable
      @param [in] error Error target
      @param [in] pnorm P-norm value for error measure. Applies the
      p-norm scaling to the metric, as in Chen, Sun and Xu,
      Mathematics of Computation, Volume 76, Number 257, January
      2007. Set to -1 to default to absolute error measure.
   */
  void pragmatic_add_field(const double *psi, const double *error, int *pnorm){
    assert(_pragmatic_mesh!=NULL);

    Mesh<double> *mesh = (Mesh<double> *)_pragmatic_mesh;

    if(_pragmatic_metric_field==NULL){

      if(((Mesh<double> *)_pragmatic_mesh)->get_number_dimensions()==2){
        MetricField2D<double> *metric_field = new MetricField2D<double>(*mesh);
        _pragmatic_metric_field = metric_field;
      }else{
        MetricField3D<double> *metric_field = new MetricField3D<double>(*mesh);
        _pragmatic_metric_field = metric_field;
      }
    }

    if(((Mesh<double> *)_pragmatic_mesh)->get_number_dimensions()==2){
      ((MetricField2D<double> *)_pragmatic_metric_field)->add_field(psi, *error, *pnorm);
      ((MetricField2D<double> *)_pragmatic_metric_field)->update_mesh();
    }else{
      ((MetricField3D<double> *)_pragmatic_metric_field)->add_field(psi, *error, *pnorm);
      ((MetricField3D<double> *)_pragmatic_metric_field)->update_mesh();
    }
  }

  /** Set the node centred metric field

      @param [in] metric Metric tensor field.
   */
  void pragmatic_set_metric(const double *metric){
    assert(_pragmatic_mesh!=NULL);
    assert(_pragmatic_metric_field==NULL);

    Mesh<double> *mesh = (Mesh<double> *)_pragmatic_mesh;

    if(_pragmatic_metric_field==NULL){
      if(((Mesh<double> *)_pragmatic_mesh)->get_number_dimensions()==2){
        MetricField2D<double> *metric_field = new MetricField2D<double>(*mesh);
        _pragmatic_metric_field = metric_field;
      }else{
        MetricField3D<double> *metric_field = new MetricField3D<double>(*mesh);
        _pragmatic_metric_field = metric_field;
      }
    }

    if(((Mesh<double> *)_pragmatic_mesh)->get_number_dimensions()==2){
      ((MetricField2D<double> *)_pragmatic_metric_field)->set_metric(metric);
      ((MetricField2D<double> *)_pragmatic_metric_field)->update_mesh();
      
      ((MetricField2D<double> *)_pragmatic_metric_field)->relax_mesh(1.0);
    }else{
      ((MetricField3D<double> *)_pragmatic_metric_field)->set_metric(metric);
      ((MetricField3D<double> *)_pragmatic_metric_field)->update_mesh();
    }
  }
  
  /** Adapt the mesh.
   */
  void pragmatic_adapt(){
    Mesh<double> *mesh = (Mesh<double> *)_pragmatic_mesh;

    const size_t ndims = mesh->get_number_dimensions();

    // See Eqn 7; X Li et al, Comp Methods Appl Mech Engrg 194 (2005) 4915-4950
    double L_up = sqrt(2.0);
    double L_low = L_up*0.5;

    if(ndims==2){
      Coarsen2D<double> coarsen(*mesh);
      Smooth2D<double> smooth(*mesh);
      Refine2D<double> refine(*mesh);
      Swapping2D<double> swapping(*mesh);

      double L_max = mesh->maximal_edge_length();

      double alpha = sqrt(2.0)/2.0;
      for(size_t i=0;i<10;i++){
        double L_ref = std::max(alpha*L_max, L_up);

        coarsen.coarsen(L_low, L_ref);
        swapping.swap(0.7);
        refine.refine(L_ref);

        L_max = mesh->maximal_edge_length();

        if((L_max-L_up)<0.01)
          break;
      }

      std::vector<int> active_vertex_map;
      mesh->defragment(&active_vertex_map);

      smooth.smooth("optimisation Linf", 10);
    }else{
      Coarsen3D<double> coarsen(*mesh);
      Smooth3D<double> smooth(*mesh);
      Refine3D<double> refine(*mesh);
      Swapping3D<double> swapping(*mesh);

      coarsen.coarsen(L_low, L_up);

      double L_max = mesh->maximal_edge_length();

      double alpha = sqrt(2.0)/2.0;
      for(size_t i=0;i<10;i++){
        double L_ref = std::max(alpha*L_max, L_up);

        refine.refine(L_ref);
        coarsen.coarsen(L_low, L_ref);
        swapping.swap(0.95);

        L_max = mesh->maximal_edge_length();

        if((L_max-L_up)<0.01)
          break;
      }

      std::vector<int> active_vertex_map;
      mesh->defragment(&active_vertex_map);

      smooth.smooth("optimisation Linf", 10);
    }
  }

  /** Get size of mesh.

      @param [out] NNodes
      @param [out] NElements
   */
  void pragmatic_get_info(int *NNodes, int *NElements){
    Mesh<double> *mesh = (Mesh<double> *)_pragmatic_mesh;

    *NNodes = mesh->get_number_nodes();
    *NElements = mesh->get_number_elements();
  }

  void pragmatic_get_coords_2d(double *x, double *y){
    size_t NNodes = ((Mesh<double> *)_pragmatic_mesh)->get_number_nodes();
    for(size_t i=0;i<NNodes;i++){
      x[i] = ((Mesh<double> *)_pragmatic_mesh)->get_coords(i)[0];
      y[i] = ((Mesh<double> *)_pragmatic_mesh)->get_coords(i)[1];
    }
  }

  void pragmatic_get_coords_3d(double *x, double *y, double *z){
    size_t NNodes = ((Mesh<double> *)_pragmatic_mesh)->get_number_nodes();
    for(size_t i=0;i<NNodes;i++){
      x[i] = ((Mesh<double> *)_pragmatic_mesh)->get_coords(i)[0];
      y[i] = ((Mesh<double> *)_pragmatic_mesh)->get_coords(i)[1];
      z[i] = ((Mesh<double> *)_pragmatic_mesh)->get_coords(i)[2];
    }
  }

  void pragmatic_get_elements(int *elements){
    const size_t ndims = ((Mesh<double> *)_pragmatic_mesh)->get_number_dimensions();
    const size_t NElements = ((Mesh<double> *)_pragmatic_mesh)->get_number_elements();
    const size_t nloc = (ndims==2)?3:4;

    for(size_t i=0;i<NElements;i++){
      const int *n=((Mesh<double> *)_pragmatic_mesh)->get_element(i);

      for(size_t j=0;j<nloc;j++){
        assert(n[j]>=0);
        elements[i*nloc+j] = n[j];
      }
    }
  }

  void pragmatic_get_lnn2gnn(int *nodes_per_partition, int *lnn2gnn){
    std::vector<int> _NPNodes, _lnn2gnn;
    ((Mesh<double> *)_pragmatic_mesh)->get_global_node_numbering(_NPNodes, _lnn2gnn);
    size_t len0 = _NPNodes.size();
    for(size_t i=0;i<len0;i++)
      nodes_per_partition[i] = _NPNodes[i];

    size_t len1 = _lnn2gnn.size();
    for(size_t i=0;i<len1;i++)
      lnn2gnn[i] = _lnn2gnn[i];
  }

  void pragmatic_get_metric(double *metric){
    if(((Mesh<double> *)_pragmatic_mesh)->get_number_dimensions()==2){
      ((MetricField2D<double> *)_pragmatic_metric_field)->get_metric(metric);
    }else{
      ((MetricField3D<double> *)_pragmatic_metric_field)->get_metric(metric);
    }
  }

  void pragmatic_finalize(){
    if(((Mesh<double> *)_pragmatic_mesh)->get_number_dimensions()==2){
      if(_pragmatic_metric_field!=NULL)
        delete (MetricField2D<double> *)_pragmatic_metric_field;
    }else{
      if(_pragmatic_metric_field!=NULL)
        delete (MetricField3D<double> *)_pragmatic_metric_field;
    }
    _pragmatic_metric_field=NULL;

    delete (Mesh<double> *)_pragmatic_mesh;
    _pragmatic_mesh=NULL;
  }
}
