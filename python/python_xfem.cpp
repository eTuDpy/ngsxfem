#ifdef NGSX_PYTHON
//#include "../ngstd/python_ngstd.hpp"
#include <python_ngstd.hpp>
#include "../xfem/xFESpace.hpp"
#include "../xfem/symboliccutbfi.hpp"
#include "../xfem/symboliccutlfi.hpp"
#include "../lsetcurving/p1interpol.hpp"
#include "../lsetcurving/calcgeomerrors.hpp"
#include "../lsetcurving/lsetrefine.hpp"
#include "../lsetcurving/projshift.hpp"
#include "../utils/error.hpp"

//using namespace ngcomp;

void ExportNgsx(py::module &m) 
{
  



  typedef PyWrapper<FESpace> PyFES;
  typedef PyWrapper<CoefficientFunction> PyCF;
  typedef GridFunction GF;
  typedef PyWrapper<GF> PyGF;
  
  py::enum_<DOMAIN_TYPE>(m, "DOMAIN_TYPE")
    .value("POS", POS)
    .value("NEG", NEG)
    .value("IF", IF)
    .export_values()
    ;

  // typedef PyWrapperDerived<CompoundFESpace, FESpace> PyCompFES;
  
  typedef PyWrapperDerived<XFESpace, FESpace> PyXFES;
  typedef PyWrapperDerived<XStdFESpace, FESpace> PyXStdFES;

  m.def("CastToXFESpace", FunctionPointer( [] (PyFES fes) -> PyXFES { return PyXFES(dynamic_pointer_cast<XFESpace>(fes.Get())); } ) );
  m.def("CastToXStdFESpace", FunctionPointer( [] (PyFES fes) -> PyXStdFES { return PyXStdFES(dynamic_pointer_cast<XStdFESpace>(fes.Get())); } ) );
  
  m.def("XToNegPos", FunctionPointer( [] (PyGF gfx, PyGF gfnegpos) { XFESpace::XToNegPos(gfx.Get(),gfnegpos.Get()); } ) );

  

  py::class_<PyXFES, PyFES>
    (m, "XFESpace")
    .def("SetLevelSet", FunctionPointer ([](PyXFES self, PyCF cf) 
                                         { self.Get()->SetLevelSet(cf.Get()); }),
         "Update information on level set function")
    .def("SetLevelSet", FunctionPointer ([](PyXFES self, PyGF gf) 
                                         { self.Get()->SetLevelSet(gf.Get()); }),
         "Update information on level set function")
    .def("SetBaseFESpace", FunctionPointer ([](PyXFES self, PyFES fes) 
                                            { self.Get()->SetBaseFESpace(fes.Get()); }),
         "Update information on base FESpace")
    .def("BaseDofOfXDof", FunctionPointer ([](PyXFES self, int i) 
                                         { return self.Get()->GetBaseDofOfXDof(i); }),
         "get corresponding dof of base FESpace")
    .def("GetNVertexDofs", FunctionPointer ([](PyXFES self) 
                                            { return self.Get()->GetNVertexDof(); }),
         "get number of x dofs at vertices")
    .def("CutElements", FunctionPointer ([](PyXFES self) 
                                         { return self.Get()->CutElements(); }),
         "get BitArray of cut elements")
    .def("CutSurfaceElements", FunctionPointer ([](PyXFES self) 
                                         { return self.Get()->CutSurfaceElements(); }),
         "get BitArray of cut surface elements")
    .def("GetDomainOfDof", FunctionPointer ([](PyXFES self, int i) 
                                         { return self.Get()->GetDomainOfDof(i); }),
         "get domain_type of degree of freedom")
    .def("GetDomainOfElement", FunctionPointer ([](PyXFES self, int i) 
                                         { return self.Get()->GetDomainOfElement(i); }),
         "get domain_type of element")
    .def("GetDomainNrs",  FunctionPointer( [] (PyXFES self, int elnr) {
               Array<DOMAIN_TYPE> domnums;
               self.Get()->GetDomainNrs( elnr, domnums );
               return domnums;
            }))
    ;


  py::class_<PyXStdFES, PyFES>
    (m, "XStdFESpace")
    .def_property_readonly("XFESpace", FunctionPointer ([](const PyXStdFES self) 
                                               { return PyXFES(dynamic_pointer_cast<XFESpace> ((*self.Get())[1])); }
                    ),
         "return XFESpace part of XStdFESpace")
    .def_property_readonly("StdFESpace", FunctionPointer ([](const PyXStdFES self) 
                                                 { return PyFES((*self.Get())[0]); }
                    ),
         "return 'standard' FESpace part of XStdFESpace")
    ;

  m.def("InterpolateToP1", FunctionPointer( [] (PyGF gf_ho, PyGF gf_p1, int heapsize)
                                              {
                                                InterpolateP1 interpol(gf_ho.Get(), gf_p1.Get());
                                                LocalHeap lh (heapsize, "InterpolateP1-Heap");
                                                interpol.Do(lh);
                                              } ),
           py::arg("gf_ho")=NULL,py::arg("gf_p1")=NULL,py::arg("heapsize")=1000000)
    ;

  m.def("InterpolateToP1", FunctionPointer( [] (PyCF coef, PyGF gf_p1, int heapsize)
                                              {
                                                InterpolateP1 interpol(coef.Get(), gf_p1.Get());
                                                LocalHeap lh (heapsize, "InterpolateP1-Heap");
                                                interpol.Do(lh);
                                              } ),
           py::arg("coef"),py::arg("gf"),py::arg("heapsize")=1000000)
    ;

  py::class_<StatisticContainer, shared_ptr<StatisticContainer>>(m, "StatisticContainer")
    .def(py::init<>())
    .def("Print", FunctionPointer ([](StatisticContainer & self, string label, string select)
                           {
                             if (select == "L1")
                               PrintConvergenceTable(self.ErrorL1Norm,label+"_L1");
                             if (select == "L2")
                               PrintConvergenceTable(self.ErrorL2Norm,label+"_L2");
                             if (select == "max")
                               PrintConvergenceTable(self.ErrorMaxNorm,label+"_max");
                             if (select == "misc")
                               PrintConvergenceTable(self.ErrorMisc,label+"_misc");
                             if (select == "all")
                             {
                               PrintConvergenceTable(self.ErrorL1Norm,label+"_L1");
                               PrintConvergenceTable(self.ErrorL2Norm,label+"_L2");
                               PrintConvergenceTable(self.ErrorMaxNorm,label+"_max");
                               PrintConvergenceTable(self.ErrorMisc,label+"_misc");
                             }
                           }),
          py::arg("label")="something",py::arg("select")="all"
      )
    ;

  m.def("CalcMaxDistance", FunctionPointer( [] (PyCF lset_ho, PyGF lset_p1, PyGF deform, int heapsize)
                                              {
                                                StatisticContainer dummy;
                                                LocalHeap lh (heapsize, "CalcDistance-Heap");
                                                if (lset_p1->GetMeshAccess()->GetDimension()==2)
                                                  CalcDistances<2>(lset_ho.Get(), lset_p1.Get(), deform.Get(),  dummy, lh, -1.0, false);
                                                else
                                                  CalcDistances<3>(lset_ho.Get(), lset_p1.Get(), deform.Get(),  dummy, lh, -1.0, false);
                                                return (double) dummy.ErrorMaxNorm[dummy.ErrorMaxNorm.Size()-1];
                                              } ),
           py::arg("lset_ho")=NULL,py::arg("lset_p1")=NULL,py::arg("deform")=NULL,py::arg("heapsize")=1000000)
    ;

  m.def("CalcDistances", FunctionPointer( [] (PyCF lset_ho, PyGF lset_p1, PyGF deform, StatisticContainer & stats, int heapsize, double refine_threshold, bool absolute)
                                              {
                                                LocalHeap lh (heapsize, "CalcDistance-Heap");
                                                if (lset_p1.Get()->GetMeshAccess()->GetDimension()==2)
                                                  CalcDistances<2>(lset_ho.Get(), lset_p1.Get(), deform.Get(),  stats, lh, refine_threshold, absolute);
                                                else
                                                  CalcDistances<3>(lset_ho.Get(), lset_p1.Get(), deform.Get(),  stats, lh, refine_threshold, absolute);
                                              } ),
           py::arg("lset_ho")=NULL,py::arg("lset_p1")=NULL,py::arg("deform")=NULL,py::arg("stats")=NULL,py::arg("heapsize")=1000000,py::arg("refine_threshold")=-1.0,py::arg("absolute")=false)
    ;

  m.def("CalcDeformationError", FunctionPointer( [] (PyCF lset_ho, PyGF lset_p1, PyGF deform, PyCF qn, StatisticContainer & stats, double lower, double upper, int heapsize)
                                              {
                                                LocalHeap lh (heapsize, "CalcDeformationError-Heap");
                                                if (lset_p1.Get()->GetMeshAccess()->GetDimension()==2)
                                                  CalcDeformationError<2>(lset_ho.Get(), lset_p1.Get(), deform.Get(), qn.Get(), stats, lh, lower, upper);
                                                else
                                                  CalcDeformationError<3>(lset_ho.Get(), lset_p1.Get(), deform.Get(), qn.Get(), stats, lh, lower, upper);
                                              } ),
           py::arg("lset_ho")=NULL,py::arg("lset_p1")=NULL,py::arg("deform")=NULL,py::arg("qn")=NULL,py::arg("stats")=NULL,py::arg("lower")=0.0,py::arg("upper")=0.0,py::arg("heapsize")=1000000)
    ;

  m.def("ProjectShift", FunctionPointer( [] (PyGF lset_ho, PyGF lset_p1, PyGF deform, PyCF qn, double lower, double upper, double threshold, int heapsize)
                                              {
                                                LocalHeap lh (heapsize, "ProjectShift-Heap");
                                                ProjectShift(lset_ho.Get(), lset_p1.Get(), deform.Get(), qn.Get(), lower, upper, threshold, lh);
                                              } ),
           py::arg("lset_ho")=NULL,py::arg("lset_p1")=NULL,py::arg("deform")=NULL,py::arg("qn")=NULL,py::arg("lower")=0.0,py::arg("upper")=0.0,py::arg("threshold")=1.0,py::arg("heapsize")=1000000)
    ;

// ProjectShift

  
  m.def("RefineAtLevelSet", FunctionPointer( [] (PyGF lset_p1, double lower, double upper, int heapsize)
                                              {
                                                LocalHeap lh (heapsize, "RefineAtLevelSet-Heap");
                                                RefineAtLevelSet(lset_p1.Get(), lower, upper, lh);
                                              } ),
           py::arg("lset_p1")=NULL,py::arg("lower")=0.0,py::arg("upper")=0.0,py::arg("heapsize")=1000000)
    ;


  m.def("CalcTraceDiff", FunctionPointer( [] (PyGF gf, PyCF coef, int intorder, int heapsize)
                                              {
                                                Array<double> errors;
                                                LocalHeap lh (heapsize, "CalcTraceDiff-Heap");
                                                if (gf.Get()->GetMeshAccess()->GetDimension() == 2)
                                                  CalcTraceDiff<2>(gf.Get(),coef.Get(),intorder,errors,lh);
                                                else 
                                                  CalcTraceDiff<3>(gf.Get(),coef.Get(),intorder,errors,lh);
                                                return errors;
                                              } ),
           py::arg("gf"),py::arg("coef"),py::arg("intorder")=6,py::arg("heapsize")=1000000)
    ;


  m.def("RefineAtLevelSet", FunctionPointer( [] (PyGF gf, double lower_lset_bound, double upper_lset_bound, int heapsize)
                                              {
                                                LocalHeap lh (heapsize, "RefineAtLevelSet-Heap");
                                                RefineAtLevelSet(gf.Get(),lower_lset_bound,upper_lset_bound,lh);
                                              } ),
           py::arg("gf"),py::arg("lower_lset_bound")=0.0,py::arg("upper_lset_bound")=0.0,py::arg("heapsize")=10000000)
    ;


  m.def("IntegrateX", 
          FunctionPointer([](PyCF lset,
                             shared_ptr<MeshAccess> ma, 
                             PyCF cf_neg,
                             PyCF cf_pos,
                             PyCF cf_interface,
                             int order, int subdivlvl, py::dict domains, int heapsize)
                          {
                            static Timer timer ("IntegrateX");
                            static Timer timercutgeom ("IntegrateX::MakeCutGeom");
                            static Timer timerevalcoef ("IntegrateX::EvalCoef");
                            RegionTimer reg (timer);
                            LocalHeap lh(heapsize, "lh-Integrate");
                            
                            Flags flags = py::extract<Flags> (domains)();
                            
                            Array<bool> tointon(3); tointon = false;
                            Array<shared_ptr<CoefficientFunction>> cf(3); cf = nullptr;
                            if (flags.GetDefineFlag("negdomain")){
                              tointon[int(NEG)] = true;
                              if (cf_neg.Get() == nullptr)
                                throw Exception("no coef for neg domain given");
                              else
                                cf[int(NEG)] = cf_neg.Get();
                            }
                            if (flags.GetDefineFlag("posdomain")){
                              tointon[int(POS)] = true;
                              if (cf_pos.Get() == nullptr)
                                throw Exception("no coef for pos domain given");
                              else
                                cf[int(POS)] = cf_pos.Get();
                            }
                            if (flags.GetDefineFlag("interface")){
                              tointon[int(IF)] = true;
                              if (cf_interface.Get() == nullptr)
                                throw Exception("no coef for interface domain given");
                              else
                                cf[int(IF)] = cf_interface.Get();
                            }

                            Vector<> domain_sum(3); // [val_neg,val_pos,val_interface]
                            domain_sum = 0.0;
                            int DIM = ma->GetDimension();
                            ma->IterateElements
                              (VOL, lh, [&] (Ngs_Element el, LocalHeap & lh)
                               {
                                 auto & trafo = ma->GetTrafo (el, lh);
                                 auto lset_eval
                                   = ScalarFieldEvaluator::Create(DIM,*(lset.Get()),trafo,lh);
                                 ELEMENT_TYPE eltype = el.GetType();
                                 timercutgeom.Start();
                                 shared_ptr<XLocalGeometryInformation> xgeom = nullptr;

                                 CompositeQuadratureRule<2> cquad2d;
                                 CompositeQuadratureRule<3> cquad3d;
                                 if (DIM == 2)
                                   xgeom = XLocalGeometryInformation::Create(eltype, ET_POINT,
                                                                             *lset_eval, cquad2d, lh,
                                                                             order, 0, subdivlvl, 0);
                                 else
                                   xgeom = XLocalGeometryInformation::Create(eltype, ET_POINT,
                                                                             *lset_eval, cquad3d, lh,
                                                                             order, 0, subdivlvl, 0);
                                 DOMAIN_TYPE element_domain = xgeom->MakeQuadRule();
                                 timercutgeom.Stop();
                                 if (element_domain == IF)
                                 {
                                   for (auto domtype : {NEG,POS})
                                   {
                                     if( ! tointon[int(domtype)] ) continue;
                                     double hsum = 0.0;
                                     // double value = 0.0;

                                     if (DIM == 2)
                                     {
                                       const QuadratureRule<2> & domain_quad = cquad2d.GetRule(domtype);
                                       IntegrationRule ir_domain (domain_quad.Size(),lh);
                                       for (int i = 0; i < ir_domain.Size(); ++i)
                                         ir_domain[i] = IntegrationPoint (&domain_quad.points[i](0),domain_quad.weights[i]);
                                       
                                       MappedIntegrationRule<2,2> mir_domain(ir_domain, trafo,lh);
                                       FlatMatrix<> values(mir_domain.Size(), 1, lh);
                                       timerevalcoef.Start();
                                       cf[int(domtype)] -> Evaluate (mir_domain, values);
                                       timerevalcoef.Stop();

                                       for (int i = 0; i < domain_quad.Size(); ++i)
                                         hsum += values(i,0) * mir_domain[i].GetWeight(); 
                                     }
                                     else
                                     {
                                       const QuadratureRule<3> & domain_quad = cquad3d.GetRule(domtype);
                                       IntegrationRule ir_domain (domain_quad.Size(),lh);
                                       for (int i = 0; i < ir_domain.Size(); ++i)
                                         ir_domain[i] = IntegrationPoint (&domain_quad.points[i](0),domain_quad.weights[i]);
                                       
                                       MappedIntegrationRule<3,3> mir_domain(ir_domain, trafo,lh);
                                       FlatMatrix<> values(mir_domain.Size(), 1, lh);
                                       timerevalcoef.Start();
                                       cf[int(domtype)] -> Evaluate (mir_domain, values);
                                       timerevalcoef.Stop();

                                       for (int i = 0; i < domain_quad.Size(); ++i)
                                         hsum += values(i,0) * mir_domain[i].GetWeight(); 
                                     }


                                     double & rsum = domain_sum(int(domtype));
                                     AsAtomic(rsum) += hsum;
                                   }

                                   if (tointon[int(IF)])
                                   {
                                     double hsum_if = 0.0;
                                     double value_if = 0.0;
                                     if (DIM == 2)
                                     {
                                       const QuadratureRuleCoDim1<2> & interface_quad(cquad2d.GetInterfaceRule());
                                       IntegrationRule ir_interface (interface_quad.Size(),lh);
                                       for (int i = 0; i < ir_interface.Size(); ++i)
                                         ir_interface[i] = IntegrationPoint (&interface_quad.points[i](0),interface_quad.weights[i]);
                                       
                                       MappedIntegrationRule<2,2> mir_interface(ir_interface, trafo,lh);
                                       FlatMatrix<> values(mir_interface.Size(), 1, lh);
                                       timerevalcoef.Start();
                                       cf[int(IF)] -> Evaluate (mir_interface, values);
                                       timerevalcoef.Stop();
                                       
                                       for (int i = 0; i < interface_quad.Size(); ++i)
                                       {
                                         MappedIntegrationPoint<2,2> & mip(mir_interface[i]);

                                         Mat<2,2> Finv = mip.GetJacobianInverse();
                                         const double absdet = mip.GetMeasure();

                                         Vec<2> nref = interface_quad.normals[i];
                                         Vec<2> normal = absdet * Trans(Finv) * nref ;
                                         double len = L2Norm(normal);
                                         const double weight = interface_quad.weights[i] * len;

                                         hsum_if += values(i,0) * weight; 
                                       }
                                     }
                                     else
                                     {
                                       
                                       const QuadratureRuleCoDim1<3> & interface_quad(cquad3d.GetInterfaceRule());
                                       IntegrationRule ir_interface (interface_quad.Size(),lh);
                                       for (int i = 0; i < ir_interface.Size(); ++i)
                                         ir_interface[i] = IntegrationPoint (&interface_quad.points[i](0),interface_quad.weights[i]);
                                       
                                       MappedIntegrationRule<3,3> mir_interface(ir_interface, trafo,lh);
                                       FlatMatrix<> values(mir_interface.Size(), 1, lh);
                                       timerevalcoef.Start();
                                       cf[int(IF)] -> Evaluate (mir_interface, values);
                                       timerevalcoef.Stop();
                                       
                                       for (int i = 0; i < interface_quad.Size(); ++i)
                                       {
                                         MappedIntegrationPoint<3,3> & mip(mir_interface[i]);

                                         Mat<3,3> Finv = mip.GetJacobianInverse();
                                         const double absdet = mip.GetMeasure();

                                         Vec<3> nref = interface_quad.normals[i];
                                         Vec<3> normal = absdet * Trans(Finv) * nref ;
                                         double len = L2Norm(normal);
                                         const double weight = interface_quad.weights[i] * len;
                                         
                                         hsum_if += values(i,0) * weight; 
                                       }
                                     }

                                     double & rsum_if = domain_sum(int(IF));
                                     AsAtomic(rsum_if) += hsum_if;
                                   }
                                 }
                                 else if( tointon[int(element_domain)] )
                                 {
                                   double hsum = 0.0;
                                   IntegrationRule ir(trafo.GetElementType(), order);
                                   BaseMappedIntegrationRule & mir = trafo(ir, lh);

                                   FlatMatrix<> values(ir.Size(), 1, lh);
                                   timerevalcoef.Start();
                                   cf[int(element_domain)] -> Evaluate (mir, values);
                                   timerevalcoef.Stop();
                                   for (int i = 0; i < values.Height(); i++)
                                     hsum += mir[i].GetWeight() * values(i,0);

                                   double & rsum = domain_sum(int(element_domain));
                                   AsAtomic(rsum) += hsum;
                                 }
                                   
                               });

                            py::dict resdict;
                            if (tointon[int(NEG)])
                              resdict["negdomain"] = py::cast(domain_sum(int(NEG)));
                            if (tointon[int(POS)])
                              resdict["posdomain"] = py::cast(domain_sum(int(POS)));
                            if (tointon[int(IF)])
                              resdict["interface"] = py::cast(domain_sum(int(IF)));

                            return resdict;
                            // bp::object result;
                            // return  bp::list(bp::object(result_vec));
                          }),
           py::arg("lset"), py::arg("mesh"), 
           py::arg("cf_neg")=PyCF(make_shared<ConstantCoefficientFunction>(0.0)), 
           py::arg("cf_pos")=PyCF(make_shared<ConstantCoefficientFunction>(0.0)),
           py::arg("cf_interface")=PyCF(make_shared<ConstantCoefficientFunction>(0.0)),
           py::arg("order")=5, py::arg("subdivlvl")=0, py::arg("domains")=py::dict(), py::arg("heapsize")=1000000)
    ;


  typedef PyWrapper<BilinearFormIntegrator> PyBFI;
  typedef PyWrapper<LinearFormIntegrator> PyLFI;
  
  m.def("SymbolicCutBFI", FunctionPointer
          ([](PyCF lset,
              DOMAIN_TYPE dt,
              int order,
              int subdivlvl,
              PyCF cf,
              VorB vb,
              bool element_boundary,
              bool skeleton,
              py::object definedon)
           -> PyBFI
           {
             
             py::extract<Region> defon_region(definedon);
             if (defon_region.check())
               vb = VorB(defon_region());

             if (vb == BND)
               throw Exception("Symbolic cuts not yet (tested) for boundaries..");

             // check for DG terms
             bool has_other = false;
             cf.Get()->TraverseTree ([&has_other] (CoefficientFunction & cf)
                               {
                                 if (dynamic_cast<ProxyFunction*> (&cf))
                                   if (dynamic_cast<ProxyFunction&> (cf).IsOther())
                                     has_other = true;
                               });
             if (has_other && !element_boundary && !skeleton)
               throw Exception("DG-facet terms need either skeleton=True or element_boundary=True");
             if (element_boundary)
               throw Exception("No Facet BFI with Symbolic cuts..");
               
             shared_ptr<BilinearFormIntegrator> bfi;
             if (!has_other && !skeleton)
               bfi = make_shared<SymbolicCutBilinearFormIntegrator> (lset.Get(), cf.Get(), dt, order, subdivlvl);
             else
               bfi = make_shared<SymbolicCutFacetBilinearFormIntegrator> (lset.Get(), cf.Get(), dt, order, subdivlvl);
             
             if (py::extract<py::list> (definedon).check())
               bfi -> SetDefinedOn (makeCArray<int> (definedon));

             if (defon_region.check())
               {
                 cout << IM(3) << "defineon = " << defon_region().Mask() << endl;
                 bfi->SetDefinedOn(defon_region().Mask());
               }
             
             return PyBFI(bfi);
           }),
           py::arg("lset"),
           py::arg("domain_type")=NEG,
           py::arg("force_intorder")=-1,
           py::arg("subdivlvl")=0,
           py::arg("form"),
           py::arg("VOL_or_BND")=VOL,
           py::arg("element_boundary")=false,
           py::arg("skeleton")=false,
           py::arg("definedon")=DummyArgument()
          );
  
  
  m.def("SymbolicCutLFI", FunctionPointer
          ([](PyCF lset,
              DOMAIN_TYPE dt,
              int order,
              int subdivlvl,
              PyCF cf,
              VorB vb,
              bool element_boundary,
              bool skeleton,
              py::object definedon)
           -> PyLFI
           {
             
             py::extract<Region> defon_region(definedon);
             if (defon_region.check())
               vb = VorB(defon_region());

             if (vb == BND)
               throw Exception("Symbolic cuts not yet (tested) for boundaries..");
               
             if (element_boundary || skeleton)
               throw Exception("No Facet LFI with Symbolic cuts..");
             
             shared_ptr<LinearFormIntegrator> lfi
               = make_shared<SymbolicCutLinearFormIntegrator> (lset.Get(), cf.Get(), dt, order, subdivlvl);
             
             if (py::extract<py::list> (definedon).check())
               lfi -> SetDefinedOn (makeCArray<int> (definedon));

             if (defon_region.check())
               {
                 cout << IM(3) << "defineon = " << defon_region().Mask() << endl;
                 lfi->SetDefinedOn(defon_region().Mask());
               }
             
             return PyLFI(lfi);
           }),
           py::arg("lset"),
           py::arg("domain_type")=NEG,
           py::arg("force_intorder")=-1,
           py::arg("subdivlvl")=0,
           py::arg("form"),
           py::arg("VOL_or_BND")=VOL,
           py::arg("element_boundary")=py::bool_(false),
           py::arg("skeleton")=py::bool_(false),
           py::arg("definedon")=DummyArgument()
          );
  
  
  // bp::def("GFCoeff", FunctionPointer( [] (shared_ptr<GridFunction> in) { return dynamic_pointer_cast<CoefficientFunction>(make_shared<GridFunctionCoefficientFunction>(in)); } ) );

  // bp::implicitly_convertible 
  //   <shared_ptr<GridFunctionCoefficientFunction>, 
  //   shared_ptr<CoefficientFunction> >(); 
  

  // void RefineAtLevelSet (PyGF gf_lset_p1, double lower_lset_bound, double upper_lset_bound, LocalHeap & lh){

 
  // bp::docstring_options local_docstring_options(true, true, false);
  
  // std::string nested_name = "comp";
  // if( bp::scope() )
  //   nested_name = bp::extract<std::string>(bp::scope().attr("__name__") + ".comp");
  typedef PyWrapperDerived<SFESpace, FESpace> PySFES;
  // py::class_<PySFES, PyFES>
  //   (m, "SFESpace")
  //   .def("Something", FunctionPointer ([](PySFES self, PyCF cf) 
  //                                      { throw Exception ("Something called"); }),
  //        "test something")
  //   .def("__init__", py::make_constructor 
  //        (FunctionPointer ([](shared_ptr<MeshAccess> ma, PyCF lset, int order, py::dict bpflags)
  //                          {
  //                            Flags flags = py::extract<Flags> (bpflags)();
  //                            shared_ptr<FESpace> ret = make_shared<SFESpace> (ma, lset.Get(), order, flags);
  //                            LocalHeap lh (1000000, "SFESpace::Update-heap", true);
  //                            ret->Update(lh);
  //                            return ret;
  //                          })));


  m.def("SFESpace", FunctionPointer
          ([](shared_ptr<MeshAccess> ma, PyCF lset, int order, py::dict bpflags)
           -> PyFES
           {
             Flags flags = py::extract<Flags> (bpflags)();
             shared_ptr<FESpace> ret = make_shared<SFESpace> (ma, lset.Get(), order, flags);
             LocalHeap lh (1000000, "SFESpace::Update-heap", true);
             ret->Update(lh);
             ret->FinalizeUpdate(lh);
             return ret;
           }));
}

PYBIND11_PLUGIN(libngsxfem_py) 
{
  py::module m("xfem", "pybind xfem");
  ExportNgsx(m);
  return m.ptr();
}
#endif // NGSX_PYTHON
