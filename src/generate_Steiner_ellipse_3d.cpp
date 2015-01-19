
/* Start of code generated by generate_Steiner_ellipse_3d.py. Warning - be careful about modifying
   any of the generated code directly.  Any changes/fixes should be done
   in the code generation script generation.
 */

#include <Eigen/Core>
#include <Eigen/Dense>

#include <generate_Steiner_ellipse_3d.h>


void pragmatic::generate_Steiner_ellipse(const double *x1, const double *x2, const double *x3, const double *x4, double *sm){
  // # From http://en.wikipedia.org/wiki/Steiner_ellipse

  Eigen::Matrix<double, 6, 6> M;

  M <<pow(x1[0] - x2[0], 2), pow(x1[1] - x2[1], 2), pow(x1[2] - x2[2], 2), (x1[1] - x2[1])*(x1[2] - x2[2]), (x1[0] - x2[0])*(x1[2] - x2[2]), (x1[0] - x2[0])*(x1[1] - x2[1]), 
pow(x2[0] - x3[0], 2), pow(x2[1] - x3[1], 2), pow(x2[2] - x3[2], 2), (x2[1] - x3[1])*(x2[2] - x3[2]), (x2[0] - x3[0])*(x2[2] - x3[2]), (x2[0] - x3[0])*(x2[1] - x3[1]), 
pow(-x1[0] + x3[0], 2), pow(-x1[1] + x3[1], 2), pow(-x1[2] + x3[2], 2), (-x1[1] + x3[1])*(-x1[2] + x3[2]), (-x1[0] + x3[0])*(-x1[2] + x3[2]), (-x1[0] + x3[0])*(-x1[1] + x3[1]), 
pow(x1[0] - x4[0], 2), pow(x1[1] - x4[1], 2), pow(x1[2] - x4[2], 2), (x1[1] - x4[1])*(x1[2] - x4[2]), (x1[0] - x4[0])*(x1[2] - x4[2]), (x1[0] - x4[0])*(x1[1] - x4[1]), 
pow(x2[0] - x4[0], 2), pow(x2[1] - x4[1], 2), pow(x2[2] - x4[2], 2), (x2[1] - x4[1])*(x2[2] - x4[2]), (x2[0] - x4[0])*(x2[2] - x4[2]), (x2[0] - x4[0])*(x2[1] - x4[1]), 
pow(x3[0] - x4[0], 2), pow(x3[1] - x4[1], 2), pow(x3[2] - x4[2], 2), (x3[1] - x4[1])*(x3[2] - x4[2]), (x3[0] - x4[0])*(x3[2] - x4[2]), (x3[0] - x4[0])*(x3[1] - x4[1]);

  Eigen::Matrix<double, 6, 1> R;
  R<<1,1,1,1,1,1;
  Eigen::Matrix<double, 6, 1> S;

  M.svd().solve(R, &S);

  sm[0] = S[0]; sm[1] = S[5]; sm[2] = S[4];
                sm[3] = S[1]; sm[4] = S[3];
                              sm[5] = S[2];
  return;
}

/* End of code generated by generate_Steiner_ellipse_3d.py. Warning - be careful about
   modifying any of the generated code directly.  Any changes/fixes
   should be done in the code generation script generation.*/
