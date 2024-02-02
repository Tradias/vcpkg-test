#include "/home/dh/deeptech-monorepo/meshrepair/wta/src/io/dassault/cadexLicense.h"

#include "lib.h"

#include <cadex/LicenseManager_Activate.h>
#include <cadex/ModelAlgo_BRepMesher.hxx>
#include <cadex/ModelAlgo_BRepMesherParameters.hxx>
#include <cadex/ModelData_BRepRepresentation.hxx>
#include <cadex/ModelData_Model.hxx>
#include <cadex/ModelData_ModelReader.hxx>
#include <cadex/ModelData_Part.hxx>
#include <cadex/ModelData_PolyRepresentation.hxx>
#include <cadex/ModelData_RepresentationMask.hxx>
#include <oneapi/tbb.h>

#include <array>

class FirstFaceGetter : public cadex::ModelData_Model::VoidElementVisitor
{
  public:
    explicit FirstFaceGetter() = default;

    void operator()(const cadex::ModelData_Part& thePart) override
    {
        ModelAlgo_BRepMesherParameters aParam;
        ModelAlgo_BRepMesher aMesher(aParam);
        auto brep = thePart.BRepRepresentation();
        if (brep.IsNull())
        {
            thePart.PolyRepresentation(cadex::ModelData_RepresentationMask::ModelData_RM_Any);
        }
        else
        {
            aMesher.Compute(brep);
        }
    }
};

int main()
{
    // tbb::task_arena arena;
    // tbb::concurrent_unordered_map<int, int> a;
    tbb::global_control g(tbb::global_control::max_allowed_parallelism, 2);

    tbb::parallel_for_each(std::array{0, 1, 2}, [](auto) {});
    lib::a();
    auto aKey = cadex::LicenseKey::Value();
    // Activate the license (aKey must be defined in cadex_license.cxx)
    if (!::CADExLicense_Activate(aKey))
    {
        return 1;
    }

    cadex::ModelData_Model aModel;
    cadex::ModelData_ModelReader aReader;
    if (!aReader.Read("", aModel))
    {
        return 1;
    }
    FirstFaceGetter aVisitor{};
    aModel.Accept(aVisitor);
    return 0;
}