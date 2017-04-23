#ifndef FILE_DIFFOPDT_HPP
#define FILE_DIFFOPDT_HPP

#include <fem.hpp>   // for ScalarFiniteElement
#include <ngstd.hpp> // for Array

namespace ngfem
{

  class DiffOpDt : public DiffOp<DiffOpDt>
  {

  public:
    enum { DIM = 1 };          // just one copy of the spaces
    enum { DIM_SPACE = 2 };    // D-dim space
    enum { DIM_ELEMENT = 2 };  // D-dim elements (in contrast to boundary elements)
    enum { DIM_DMAT = 1 };     // D-matrix
    enum { DIFFORDER = 1 };    // minimal differential order (to determine integration order)

    template <typename FEL, typename MIP, typename MAT>
    static void GenerateMatrix (const FEL & bfel, const MIP & sip,
                                MAT & mat, LocalHeap & lh);
  };


}

#endif
