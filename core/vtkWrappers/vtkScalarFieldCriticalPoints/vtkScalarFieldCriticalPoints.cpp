#include                  <vtkScalarFieldCriticalPoints.h>

vtkStandardNewMacro(vtkScalarFieldCriticalPoints)

vtkScalarFieldCriticalPoints::vtkScalarFieldCriticalPoints(){

  // init
  PredefinedOffset = false;
  VertexIds = true;
  VertexScalars = true;

  ScalarFieldId = 0;
  OffsetFieldId = -1;
  OffsetField = "OutputOffsetScalarField";
}

vtkScalarFieldCriticalPoints::~vtkScalarFieldCriticalPoints(){

}

int vtkScalarFieldCriticalPoints::doIt(vector<vtkDataSet *> &inputs, 
  vector<vtkDataSet *> &outputs){

  Memory m;
  Timer t;

  vtkDataSet *input = inputs[0];
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(outputs[0]);
  
  Triangulation *triangulation = vtkTriangulation::getTriangulation(input);
  
  if(!triangulation)
    return -1;
  
  // in the following, the target scalar field of the input is replaced in the 
  // variable 'output' with the result of the computation.
  // if your wrapper produces an output of the same type of the input, you 
  // should proceed in the same way.
  vtkDataArray *inputScalarField = NULL;
  vtkDataArray *offsetField = NULL;
  
  if(ScalarField.length()){
    inputScalarField = input->GetPointData()->GetArray(ScalarField.data());
  }
  else{
    inputScalarField = input->GetPointData()->GetArray(ScalarFieldId);
  }
  
  if(!inputScalarField)
    return -1;
  
  {
    stringstream msg;
    msg << "[vtkScalarFieldCriticalPoints] Starting computation on field `"
      << inputScalarField->GetName() << "'..." << endl;
    dMsg(cout, msg.str(), infoMsg);
  }
 
  if(OffsetFieldId != -1){
    offsetField = input->GetPointData()->GetArray(OffsetFieldId);
    if(offsetField){
      PredefinedOffset = true;
      OffsetField = offsetField->GetName();
    }
  }

  if(PredefinedOffset){
    if(OffsetField.length()){
      
      offsetField = input->GetPointData()->GetArray(OffsetField.data());
      // not good... in the future, we want to use the pointer itself...
      sosOffsets_.resize(offsetField->GetNumberOfTuples());
      for(int i = 0; i < offsetField->GetNumberOfTuples(); i++){
        int offset = 0;
        offset = offsetField->GetTuple1(i);
        sosOffsets_[i] = offset;
      }
    }
  }
  
  switch(inputScalarField->GetDataType()){
    vtkTemplateMacro((
      {
        ScalarFieldCriticalPoints<VTK_TT> criticalPoints;
        criticalPoints.setupTriangulation(triangulation);
      }
    ));
  }
    
  int domainDimension = triangulation->getCellVertexNumber(0) - 1;
 
  switch(inputScalarField->GetDataType()){
    vtkTemplateMacro((
      {
        ScalarFieldCriticalPoints<VTK_TT> criticalPoints;
       
        criticalPoints.setWrapper(this);
        criticalPoints.setDebugLevel(Debug::infoMsg);
        criticalPoints.setDomainDimension(domainDimension);
        // set up input
        // 1 -- vertex values
        criticalPoints.setScalarValues(inputScalarField->GetVoidPointer(0));
        criticalPoints.setVertexNumber(input->GetNumberOfPoints());
        
        // 2 -- set offsets (here, let the baseCode class fill it for us)
        criticalPoints.setSosOffsets(&sosOffsets_);
        
        // 3 -- set the connectivity
        criticalPoints.setupTriangulation(triangulation);
        
        // set up output
        criticalPoints.setOutput(&criticalPoints_);
        
        criticalPoints.execute();
      }
    ));
  }

  // allocate the output
  vtkSmartPointer<vtkCharArray> vertexTypes = 
    vtkSmartPointer<vtkCharArray>::New();
  
  vertexTypes->SetNumberOfComponents(1);
  vertexTypes->SetNumberOfTuples(criticalPoints_.size());
  vertexTypes->SetName("Critical Type");
  
  vtkSmartPointer<vtkPoints> pointSet = vtkSmartPointer<vtkPoints>::New();
  pointSet->SetNumberOfPoints(criticalPoints_.size());
  double p[3];
  for(int i = 0; i < (int) criticalPoints_.size(); i++){
    input->GetPoint(criticalPoints_[i].first, p);
    pointSet->SetPoint(i, p);
    vertexTypes->SetTuple1(i, (float) criticalPoints_[i].second);
  }
  output->SetPoints(pointSet);
  output->GetPointData()->AddArray(vertexTypes);
  
  if(VertexIds){
    vtkSmartPointer<vtkIntArray> vertexIds = 
      vtkSmartPointer<vtkIntArray>::New();
    vertexIds->SetNumberOfComponents(1);
    vertexIds->SetNumberOfTuples(criticalPoints_.size());
    vertexIds->SetName("Vertex Ids");
    
    for(int i = 0; i < (int) criticalPoints_.size(); i++){
      vertexIds->SetTuple1(i, (float) criticalPoints_[i].first);
    }
      
    output->GetPointData()->AddArray(vertexIds);
  }
  else{
    output->GetPointData()->RemoveArray("Vertex Ids");
  }
  
  if(VertexScalars){
    for(int i = 0; i < input->GetPointData()->GetNumberOfArrays(); i++){
      
      vtkDataArray *scalarField = input->GetPointData()->GetArray(i);
      
      switch(scalarField->GetDataType()){
        
        case VTK_CHAR:
          {
            vtkSmartPointer<vtkCharArray> scalarArray = 
              vtkSmartPointer<vtkCharArray>::New();
            scalarArray->SetNumberOfComponents(
              scalarField->GetNumberOfComponents());
            scalarArray->SetNumberOfTuples(criticalPoints_.size());
            scalarArray->SetName(scalarField->GetName());
            double *value = new double[scalarField->GetNumberOfComponents()];
            for(int j = 0; j < (int) criticalPoints_.size(); j++){
              scalarField->GetTuple(
                criticalPoints_[j].first, value);
              scalarArray->SetTuple(j, value);
            }
            output->GetPointData()->AddArray(scalarArray);
            delete value;
          }
          break;
          
        case VTK_DOUBLE:
          {
            vtkSmartPointer<vtkDoubleArray> scalarArray = 
              vtkSmartPointer<vtkDoubleArray>::New();
            scalarArray->SetNumberOfComponents(
              scalarField->GetNumberOfComponents());
            scalarArray->SetNumberOfTuples(criticalPoints_.size());
            scalarArray->SetName(scalarField->GetName());
            double *value = new double[scalarField->GetNumberOfComponents()];
            for(int j = 0; j < (int) criticalPoints_.size(); j++){
              scalarField->GetTuple(
                criticalPoints_[j].first, value);
              scalarArray->SetTuple(j, value);
            }
            output->GetPointData()->AddArray(scalarArray);
            delete value;
          }
          break;
          
        case VTK_FLOAT:
          {
            vtkSmartPointer<vtkFloatArray> scalarArray = 
              vtkSmartPointer<vtkFloatArray>::New();
            scalarArray->SetNumberOfComponents(
              scalarField->GetNumberOfComponents());
            scalarArray->SetNumberOfTuples(criticalPoints_.size());
            scalarArray->SetName(scalarField->GetName());
            double *value = new double[scalarField->GetNumberOfComponents()];
            for(int j = 0; j < (int) criticalPoints_.size(); j++){
              scalarField->GetTuple(
                criticalPoints_[j].first, value);
              scalarArray->SetTuple(j, value);
            }
            output->GetPointData()->AddArray(scalarArray);
            delete value;
          }
          break;
          
        case VTK_INT:
          {
            vtkSmartPointer<vtkIntArray> scalarArray = 
              vtkSmartPointer<vtkIntArray>::New();
            scalarArray->SetNumberOfComponents(
              scalarField->GetNumberOfComponents());
            scalarArray->SetNumberOfTuples(criticalPoints_.size());
            scalarArray->SetName(scalarField->GetName());
            double *value = new double[scalarField->GetNumberOfComponents()];
            for(int j = 0; j < (int) criticalPoints_.size(); j++){
              scalarField->GetTuple(
                criticalPoints_[j].first, value);
              scalarArray->SetTuple(j, value);
            }
            output->GetPointData()->AddArray(scalarArray);
            delete value;
          }
          break;
          
        default:
          {
            stringstream msg;
            msg << "[vtkScalarFieldCriticalPoints] Scalar attachment: "
              << "unsupported data type :(" << endl;
            dMsg(cerr, msg.str(), detailedInfoMsg);
          }
          break;
      }
    }
  }
  else{
    for(int i = 0; i < input->GetPointData()->GetNumberOfArrays(); i++){
      output->GetPointData()->RemoveArray(
        input->GetPointData()->GetArray(i)->GetName());
    }
  }
  
  {
    stringstream msg;
    msg << "[vtkScalarFieldCriticalPoints] Memory usage: " 
      << m.getElapsedUsage() 
      << " MB." << endl;
    dMsg(cout, msg.str(), 2);
  }
 
  return 0;
}