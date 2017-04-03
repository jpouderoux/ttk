#include<vtkDistanceField.h>

vtkStandardNewMacro(vtkDistanceField)

  vtkDistanceField::vtkDistanceField():
    identifiers_{}
{
  SetNumberOfInputPorts(2);
  
  triangulation_ = NULL;
}

vtkDistanceField::~vtkDistanceField(){
}

int vtkDistanceField::FillInputPortInformation(int port, vtkInformation* info){
  if(port == 0)
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataSet");
  if(port == 1)
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPointSet");

  return 1;
}

int vtkDistanceField::getTriangulation(vtkDataSet* input){
  
  triangulation_ = vtkTriangulation::getTriangulation(input);
  triangulation_->setWrapper(this);
  distanceField_.setWrapper(this);
  
  distanceField_.setupTriangulation(triangulation_);
  Modified();
  
#ifndef withKamikaze
  // allocation problem
  if(triangulation_->isEmpty()){
    cerr << "[vtkDistanceField] Error : vtkTriangulation allocation problem." << endl;
    return -1;
  }
#endif

  return 0;
}

int vtkDistanceField::getIdentifiers(vtkDataSet* input){
  if(VertexIdentifierScalarFieldName.length())
    identifiers_=input->GetPointData()->GetArray(VertexIdentifierScalarFieldName.data());

#ifndef withKamikaze
  // allocation problem
  if(!identifiers_){
    cerr << "[vtkDistanceField] Error : wrong vertex identifiers." << endl;
    return -1;
  }
#endif

  return 0;
}

int vtkDistanceField::doIt(vector<vtkDataSet *> &inputs, 
  vector<vtkDataSet *> &outputs){
  
  Memory m;
  
  vtkDataSet *domain = vtkDataSet::SafeDownCast(inputs[0]);
  vtkPointSet *sources = vtkPointSet::SafeDownCast(inputs[1]);
  vtkDataSet *output = vtkDataSet::SafeDownCast(outputs[0]);

  int ret{};

  ret=getTriangulation(domain);
#ifndef withKamikaze
  if(ret){
    cerr << "[vtkDistanceField] Error : wrong triangulation." << endl;
    return -1;
  }
#endif

  ret=getIdentifiers(sources);
#ifndef withKamikaze
  if(ret){
    cerr << "[vtkDistanceField] Error : wrong identifiers." << endl;
    return -2;
  }
#endif

  const int numberOfPointsInDomain=domain->GetNumberOfPoints();
#ifndef withKamikaze
  if(!numberOfPointsInDomain){
    cerr << "[vtkDistanceField] Error : domain has no points." << endl;
    return -3;
  }
#endif

  const int numberOfPointsInSources=sources->GetNumberOfPoints();
#ifndef withKamikaze
  if(!numberOfPointsInSources){
    cerr << "[vtkDistanceField] Error : sources have no points." << endl;
    return -4;
  }
#endif

  vtkSmartPointer<vtkIntArray> origin=vtkSmartPointer<vtkIntArray>::New();
  if(origin){
    origin->SetNumberOfComponents(1);
    origin->SetNumberOfTuples(numberOfPointsInDomain);
    origin->SetName("SeedVertexIdentifier");
  }
#ifndef withKamikaze
  else{
    cerr << "[vtkDistanceField] Error : vtkIntArray allocation problem." << endl;
    return -5;
  }
#endif

  vtkSmartPointer<vtkIntArray> seg=vtkSmartPointer<vtkIntArray>::New();
  if(seg){
    seg->SetNumberOfComponents(1);
    seg->SetNumberOfTuples(numberOfPointsInDomain);
    seg->SetName("SeedIdentifier");
  }
#ifndef withKamikaze
  else{
    cerr << "[vtkDistanceField] Error : vtkIntArray allocation problem." << endl;
    return -6;
  }
#endif

  
  distanceField_.setVertexNumber(numberOfPointsInDomain);
  distanceField_.setSourceNumber(numberOfPointsInSources);
  
distanceField_.setVertexIdentifierScalarFieldPointer(identifiers_->
GetVoidPointer(0));
  distanceField_.setOutputIdentifiers(origin->GetVoidPointer(0));
  distanceField_.setOutputSegmentation(seg->GetVoidPointer(0));

  vtkDataArray* distanceScalars{};
  switch(OutputScalarFieldType){
    case DistanceType::Float:
      distanceScalars=vtkFloatArray::New();
      if(distanceScalars){
        distanceScalars->SetNumberOfComponents(1);
        distanceScalars->SetNumberOfTuples(numberOfPointsInDomain);
        distanceScalars->SetName(OutputScalarFieldName.data());
      }
#ifndef withKamikaze
      else{
        cerr << "[vtkDistanceField] Error : vtkFloatArray allocation problem." 
<< endl;
        return -7;
      }
#endif

      
distanceField_.setOutputScalarFieldPointer(distanceScalars->GetVoidPointer(0));
      ret=distanceField_.execute<float>();
      break;

    case DistanceType::Double:
      distanceScalars=vtkDoubleArray::New();
      if(distanceScalars){
        distanceScalars->SetNumberOfComponents(1);
        distanceScalars->SetNumberOfTuples(numberOfPointsInDomain);
        distanceScalars->SetName(OutputScalarFieldName.data());
      }
#ifndef withKamikaze
      else{
        cerr << "[vtkDistanceField] Error : vtkDoubleArray allocation problem." 
<< endl;
        return -8;
      }
#endif

      
distanceField_.setOutputScalarFieldPointer(distanceScalars->GetVoidPointer(0));
      ret=distanceField_.execute<double>();
      break;

    default:
#ifndef withKamikaze
      cerr << "[vtkDistanceField] Error : Scalar field type problem." << endl;
      return -9;
#endif
      break;
  }

#ifndef withKamikaze
  // something wrong in baseCode
  if(ret){
    cerr << "[vtkDistanceField] DistanceField.execute() error code : " << ret << endl;
    return -10;
  }
#endif

  // update result
  output->ShallowCopy(domain);
  output->GetPointData()->AddArray(distanceScalars);
  output->GetPointData()->AddArray(origin);
  output->GetPointData()->AddArray(seg);
  distanceScalars->Delete();

  {
    stringstream msg;
    msg << "[vtkDistanceField] Memory usage: " << m.getElapsedUsage()
      << " MB." << endl;
    dMsg(cout, msg.str(), memoryMsg);
  }
  
  return 0;
}