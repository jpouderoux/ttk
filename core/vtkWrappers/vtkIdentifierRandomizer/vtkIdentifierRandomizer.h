/// \ingroup vtkWrappers
/// \class vtkIdentifierRandomizer
/// \author Julien Tierny <julien.tierny@lip6.fr>
/// \date March 2017.
///
/// \brief TTK VTK-filter that randomly shuffles segmentation identifiers.
///
/// \param Input Input scalar field (vtkDataSet)
/// \param Output Output scalar field (vtkDataSet)
///
/// This filter can be used as any other VTK filter (for instance, by using the 
/// sequence of calls SetInputData(), Update(), GetOutput()).
///
/// See the related ParaView example state files for usage examples within a 
/// VTK pipeline.
///
/// \sa ttk::IdentifierRandomizer
#pragma once

// ttk code includes
#include                  <ttkWrapper.h>

// VTK includes -- to adapt
#include                  <vtkCellData.h>
#include                  <vtkCharArray.h>
#include                  <vtkDataArray.h>
#include                  <vtkDataSet.h>
#include                  <vtkDataSetAlgorithm.h>
#include                  <vtkDoubleArray.h>
#include                  <vtkFiltersCoreModule.h>
#include                  <vtkFloatArray.h>
#include                  <vtkInformation.h>
#include                  <vtkIntArray.h>
#include                  <vtkObjectFactory.h>
#include                  <vtkPointData.h>
#include                  <vtkSmartPointer.h>

// in this example, this wrapper takes a data-set on the input and produces a 
// data-set on the output - to adapt.
// see the documentation of the vtkAlgorithm class to decide from which VTK 
// class your wrapper should inherit.
class VTKFILTERSCORE_EXPORT vtkIdentifierRandomizer 
  : public vtkDataSetAlgorithm, public Wrapper{

  public:
    
    static vtkIdentifierRandomizer* New();
    vtkTypeMacro(vtkIdentifierRandomizer, vtkDataSetAlgorithm)
    
    // default ttk setters
    vtkSetMacro(debugLevel_, int);
    
    void SetThreadNumber(int threadNumber){\
      ThreadNumber = threadNumber;\
      SetThreads();\
    }\
    void SetUseAllCores(bool onOff){
      UseAllCores = onOff;
      SetThreads();
    }
    // end of default ttk setters
    
        
    vtkSetMacro(ScalarField, string);
    vtkGetMacro(ScalarField, string);

  protected:
   
    vtkIdentifierRandomizer(){
      
        // init
      outputScalarField_ = NULL;
      
      UseAllCores = false;
    }
    
    ~vtkIdentifierRandomizer(){
      if(outputScalarField_){
        outputScalarField_->Delete();
      }
    };
    
    TTK_SETUP();
    
    
  private:
    
    string                ScalarField;
    vtkDataArray          *outputScalarField_;
    
};