/******************************************************************************

  This source file is part of the TEM tomography project.

  Copyright Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/
#include "ModuleSlice.h"

#include "DataSource.h"
#include "pqProxiesWidget.h"
#include "Utilities.h"

#include "vtkAlgorithm.h"
#include "vtkImageMapToColors.h"
#include "vtkImagePlaneWidget.h"
#include "vtkNew.h"
#include "vtkProperty.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMViewProxy.h"
#include "vtkPVArrayInformation.h"
#include "vtkScalarsToColors.h"

namespace TEM
{
//-----------------------------------------------------------------------------
ModuleSlice::ModuleSlice(QObject* parentObject) : Superclass(parentObject)
{
}

//-----------------------------------------------------------------------------
ModuleSlice::~ModuleSlice()
{
  this->finalize();
}

//-----------------------------------------------------------------------------
QIcon ModuleSlice::icon() const
{
  return QIcon(":/pqWidgets/Icons/pqSlice24.png");
}

//-----------------------------------------------------------------------------
bool ModuleSlice::initialize(DataSource* dataSource, vtkSMViewProxy* view)
{
  if (!this->Superclass::initialize(dataSource, view))
    {
    return false;
    }

  vtkSMSourceProxy* producer = dataSource->producer();
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;

  vtkSMSessionProxyManager* pxm = dataSource->producer()->GetSessionProxyManager();

  // Create the pass through filter.
  vtkSmartPointer<vtkSMProxy> proxy;
  proxy.TakeReference(pxm->NewProxy("filters", "PassThrough"));

  this->PassThrough = vtkSMSourceProxy::SafeDownCast(proxy);
  Q_ASSERT(this->PassThrough);
  controller->PreInitializeProxy(this->PassThrough);
  vtkSMPropertyHelper(this->PassThrough, "Input").Set(dataSource->producer());
  controller->PostInitializeProxy(this->PassThrough);
  controller->RegisterPipelineProxy(this->PassThrough);


  bool haveInteractor = false;
  bool haveWidgetInput = false;

  //get the current Interactor
  vtkRenderWindowInteractor* rwi = view->GetRenderWindow()->GetInteractor();
  vtkAlgorithm* passThrough = vtkAlgorithm::SafeDownCast(
                                   this->PassThrough->GetClientSideObject());

  // Create the representation for it.
  this->Widget = vtkSmartPointer<vtkImagePlaneWidget>::New();

  if(rwi)
    {
    //set the interactor on the widget to be what the current
    //render window is using
    this->Widget->SetInteractor( rwi );
    haveInteractor = true;
    }

  vtkSmartPointer<vtkProperty> ipwProp =
    vtkSmartPointer<vtkProperty>::New();

  //setup the rest of the widget
  this->Widget->RestrictPlaneToVolumeOff();

  {
  double color[3] = {1, 0, 0};
  this->Widget->GetPlaneProperty()->SetColor(color);
  }

  this->Widget->SetTexturePlaneProperty(ipwProp);
  this->Widget->TextureInterpolateOff();
  this->Widget->SetResliceInterpolateToLinear();

  //setup the transfer function manager.

  // ColorArrayName
  const char* propertyName = producer->GetDataInformation()->
                                       GetPointDataInformation()->
                                       GetArrayInformation(0)->
                                       GetName();

  vtkNew<vtkSMTransferFunctionManager> tfm;
  vtkSMProxy* transferProxy = tfm->GetColorTransferFunction(propertyName,pxm);

  vtkScalarsToColors* stc =  vtkScalarsToColors ::SafeDownCast(transferProxy->GetClientSideObject());
  if(stc)
    {
    std::cout << "setting up color map to property " << propertyName << std::endl;
    vtkNew<vtkImageMapToColors> colorMap;
    colorMap->SetInputConnection(passThrough->GetOutputPort());
    colorMap->SetLookupTable(stc);
    colorMap->SetOutputFormatToRGBA();
    colorMap->PassAlphaToOutputOn();
    this->Widget->SetColorMap(colorMap.GetPointer());
    }

  if(passThrough)
    {
    //set the input connection to the local pass through filter
    this->Widget->SetInputConnection(passThrough->GetOutputPort());
    haveWidgetInput = true;
    }


  this->Widget->On();
  this->Widget->InteractionOn();

  Q_ASSERT(this->Widget);
  Q_ASSERT(rwi);
  Q_ASSERT(passThrough);
  return haveInteractor && haveWidgetInput;
}

//-----------------------------------------------------------------------------
bool ModuleSlice::finalize()
{
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
  controller->UnRegisterProxy(this->PassThrough);

  this->PassThrough = NULL;
  return true;
}

//-----------------------------------------------------------------------------
bool ModuleSlice::setVisibility(bool val)
{
  Q_ASSERT(this->Widget);
  this->Widget->SetEnabled(val ? 1 : 0);
  return true;
}

//-----------------------------------------------------------------------------
bool ModuleSlice::visibility() const
{
  Q_ASSERT(this->Widget);
  return this->Widget->GetEnabled() != 0;
}

//-----------------------------------------------------------------------------
void ModuleSlice::addToPanel(pqProxiesWidget* panel)
{
  Q_ASSERT(this->Widget);

  // vtkSMProxy* lut = vtkSMPropertyHelper(this->Representation, "LookupTable").GetAsProxy();
  // Q_ASSERT(lut);

  // QStringList list;
  // list
  //   << "Mapping Data"
  //   << "EnableOpacityMapping"
  //   << "RGBPoints"
  //   << "ScalarOpacityFunction"
  //   << "UseLogScale";
  // panel->addProxy(lut, "Color Map", list, true);
  // this->Superclass::addToPanel(panel);
}

//-----------------------------------------------------------------------------
bool ModuleSlice::serialize(pugi::xml_node& ns) const
{
  // vtkSMProxy* lut = vtkSMPropertyHelper(this->Representation, "LookupTable").GetAsProxy();
  // vtkSMProxy* sof = vtkSMPropertyHelper(this->Representation, "ScalarOpacityFunction").GetAsProxy();
  // Q_ASSERT(lut && sof);

  // QStringList reprProperties;
  // reprProperties
  //   << "SliceMode"
  //   << "Slice"
  //   << "Visibility";
  // pugi::xml_node nodeR = ns.append_child("Representation");
  // pugi::xml_node nodeL = ns.append_child("LookupTable");
  // pugi::xml_node nodeS = ns.append_child("ScalarOpacityFunction");
  // return (TEM::serialize(this->Representation, nodeR, reprProperties) &&
  //   TEM::serialize(lut, nodeL) &&
  //   TEM::serialize(sof, nodeS));
  return false;
}

//-----------------------------------------------------------------------------
bool ModuleSlice::deserialize(const pugi::xml_node& ns)
{
  return false;
  // vtkSMProxy* lut = vtkSMPropertyHelper(this->Representation, "LookupTable").GetAsProxy();
  // vtkSMProxy* sof = vtkSMPropertyHelper(this->Representation, "ScalarOpacityFunction").GetAsProxy();
  // if (TEM::deserialize(this->Representation, ns.child("Representation")))
  //   {
  //   vtkSMPropertyHelper(this->Representation, "ScalarOpacityFunction").Set(sof);
  //   this->Representation->UpdateVTKObjects();
  //   }
  // else
  //   {
  //   return false;
  //   }
  // if (TEM::deserialize(lut, ns.child("LookupTable")))
  //   {
  //   vtkSMPropertyHelper(lut, "ScalarOpacityFunction").Set(sof);
  //   lut->UpdateVTKObjects();
  //   }
  // else
  //   {
  //   return false;
  //   }
  // if (TEM::deserialize(sof, ns.child("ScalarOpacityFunction")))
  //   {
  //   sof->UpdateVTKObjects();
  //   }
  // else
  //   {
  //   return false;
  //   }
  // return true;
}

}
