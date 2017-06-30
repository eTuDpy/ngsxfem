#define FILE_DIFFOPDT_CPP
#include "diffopDt.hpp"
#include <diffop_impl.hpp>
#include "SpaceTimeFE.hpp"


namespace ngfem
{

  template <typename FEL, typename MIP, typename MAT>
  void DiffOpDt::GenerateMatrix (const FEL & bfel, const MIP & mip,
                                             MAT & mat, LocalHeap & lh)
  {

      const SpaceTimeFE & scafe =
              dynamic_cast<const SpaceTimeFE & > (bfel);
      const int ndof = scafe.GetNDof();

      FlatVector<> dtshape (ndof,lh);
      IntegrationPoint ip(mip.IP());
      scafe.CalcDtShape(ip,dtshape);
      mat = 0.0;
      mat.Row(0) = dtshape;


    }

  template class T_DifferentialOperator<DiffOpDt>;


  template <int time>
  template <typename FEL, typename MIP, typename MAT>
  void DiffOpFixt<time>::GenerateMatrix (const FEL & bfel, const MIP & mip,
                                             MAT & mat, LocalHeap & lh)
  {

      const SpaceTimeFE & scafe =
              dynamic_cast<const SpaceTimeFE & > (bfel);
      const int ndof = scafe.GetNDof();

      FlatVector<> shape (ndof,lh);
      IntegrationPoint ip(mip.IP()(0),mip.IP()(1),time);
      scafe.CalcShape(ip,shape);
      mat = 0.0;
      mat.Row(0) = shape;


   }

  template class T_DifferentialOperator<DiffOpFixt<0>>;
  template class T_DifferentialOperator<DiffOpFixt<1>>;


  void DiffOpFixAnyTime ::
  CalcMatrix (const FiniteElement & bfel,
              const BaseMappedIntegrationPoint & bmip,
              SliceMatrix<double,ColMajor> mat,
              LocalHeap & lh) const
  {
    const MappedIntegrationPoint<DIM_ELEMENT,DIM_SPACE> & mip =
      static_cast<const MappedIntegrationPoint<DIM_ELEMENT,DIM_SPACE>&> (bmip);

    const SpaceTimeFE & scafe =
            dynamic_cast<const SpaceTimeFE & > (bfel);
    const int ndof = scafe.GetNDof();

    FlatVector<> shape (ndof,lh);
    IntegrationPoint ip(mip.IP()(0),mip.IP()(1),time);
    scafe.CalcShape(ip,shape);
    mat = 0.0;
    mat.Row(0) = shape;
  }



}


