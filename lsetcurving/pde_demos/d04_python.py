from ngsolve.fem import *
from ngsolve.comp import *
from ngsolve.solve import *
from ngsolve.la import *

#from libngsxfem_xfem import *
#from libngsxfem_lsetcurving import *

from libngsxfem_py.xfem import *
#from libngsxfem_levelset import *
# from libngsxfem_xstokes import *


from numpy import linspace
from timeit import Timer

mesh = Mesh("d01_testgeom.vol.gz")

dim = 2

lset_lower_bound = 0
lset_upper_bound = 0

order_deform = 3
order_qn = 3
order_lset = 3

threshold = 10.2

lset_stats = StatisticContainer()
deform_stats = StatisticContainer()

# x = CoordCF(0)
# y = CoordCF(1)
# z = CoordCF(2)

R = 2.0/3.0
levelset = VariableCF("sqrt(x*x+y*y+z*z") - R


v_ho = FESpace("h1ho", mesh, order=order_lset)
lset_ho = GridFunction (v_ho, "h.o. level set fct.")
lset_ho.Set(levelset)

v_qn = FESpace("h1ho", mesh, order=order_qn, flags = { "vec" : True })
qn = GridFunction(v_qn, "h.o. quasi normal")
qn.Set(lset_ho.Deriv())


v_p1 = FESpace("h1ho", mesh, order=1)
lset_p1 = GridFunction (v_p1, "P1 level set fct.")

InterpolateToP1(lset_ho,lset_p1)

v_def = FESpace("h1ho", mesh, order=order_deform, flags = { "vec" : True })
deform = GridFunction(v_def, "Mesh Deformation")

f_def = LinearForm (v_def)
f_def += LFI (name = "shiftsource_6", coef = [lset_p1, levelset, ConstantCF(threshold), ConstantCF(lset_lower_bound), ConstantCF(lset_upper_bound), qn])

f_def.Assemble()

a_def = BilinearForm (v_def)
for d in range(0,dim):
    a_def += BlockBFI( BFI (name = "restrictedmass_4", coef = [ConstantCF(1.0), lset_p1, ConstantCF(lset_lower_bound), ConstantCF(lset_upper_bound)]), dim, d);

a_def.Assemble()

inv_def = a_def.mat.Inverse(v_def.FreeDofs())
deform.vec.data = inv_def * f_def.vec

# calculate errors and store data
CalcDistances(lset_ho,lset_p1,deform,lset_stats)

CalcDeformationError(lset_ho=lset_ho,lset_p1=lset_p1,deform=deform,qn=qn,stats=deform_stats,lower=lset_lower_bound,upper=lset_upper_bound)

lset_stats.Print("lset_dist","max")
deform_stats.Print("deform_error","max")

# mark elements in band of interest for refinement
RefineAtLevelSet(lset_p1,lset_lower_bound,lset_upper_bound)
