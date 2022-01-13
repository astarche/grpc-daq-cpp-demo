
#include <grpcpp/grpcpp.h>
#include <nidaqmx.grpc.pb.h>

#include <iostream>
#include <memory>
#include <string>

namespace {
using StubPtr = std::unique_ptr<nidaqmx_grpc::NiDAQmx::Stub>;
std::unique_ptr<grpc::ClientContext> create_context()
{
  return std::make_unique<grpc::ClientContext>();
}

template <typename TResponse>
void raise_if_error(StubPtr& stub, const TResponse& response)
{
  if (response.status()) {
    auto get_error_request = nidaqmx_grpc::GetErrorStringRequest{};
    get_error_request.set_error_code(response.status());
    auto get_error_response = nidaqmx_grpc::GetErrorStringResponse{};
    auto client_context = create_context();
    stub->GetErrorString(client_context.get(), get_error_request, &get_error_response);
    std::cout << get_error_response.error_string() << std::endl;
    throw std::exception{};
  }
}

class DaqTask {
 public:
  DaqTask(const std::shared_ptr<grpc::Channel>& channel, const std::string& physical_channels)
      : stub_(nidaqmx_grpc::NiDAQmx::NewStub(channel)), physical_channels_(physical_channels)
  {
    auto stub = nidaqmx_grpc::NiDAQmx::NewStub(channel);

    auto client_context = create_context();
    auto create_request = nidaqmx_grpc::CreateTaskRequest{};
    create_request.set_session_name("daq_grpc");
    auto create_response = nidaqmx_grpc::CreateTaskResponse{};
    stub_->CreateTask(client_context.get(), create_request, &create_response);
    raise_if_error(stub, create_response);
  }

  ~DaqTask()
  {
    auto clear_task_request = nidaqmx_grpc::ClearTaskRequest{};
    clear_task_request.mutable_task()->set_name("daq_grpc");
    auto clear_task_response = nidaqmx_grpc::ClearTaskResponse{};
    auto client_context = create_context();
    stub_->ClearTask(client_context.get(), clear_task_request, &clear_task_response);
    raise_if_error(stub_, clear_task_response);
  }

  static std::unique_ptr<DaqTask> create(const std::shared_ptr<grpc::Channel>& channel, const std::string& physical_channels)
  {
    return std::make_unique<DaqTask>(channel, physical_channels);
  }

  void configure()
  {
    auto create_channel_request = nidaqmx_grpc::CreateAIVoltageChanRequest{};
    create_channel_request.mutable_task()->set_name("daq_grpc");
    create_channel_request.set_physical_channel(physical_channels_);
    create_channel_request.set_min_val(0.0);
    create_channel_request.set_max_val(10.0);
    create_channel_request.set_units(nidaqmx_grpc::VoltageUnits2::VOLTAGE_UNITS2_VOLTS);
    create_channel_request.set_terminal_config(nidaqmx_grpc::InputTermCfgWithDefault::INPUT_TERM_CFG_WITH_DEFAULT_CFG_DEFAULT);
    auto create_channel_response = nidaqmx_grpc::CreateAIVoltageChanResponse{};
    auto client_context = create_context();
    stub_->CreateAIVoltageChan(client_context.get(), create_channel_request, &create_channel_response);
    raise_if_error(stub_, create_channel_response);

    auto cfg_clock_request = nidaqmx_grpc::CfgSampClkTimingRequest{};
    cfg_clock_request.mutable_task()->set_name("daq_grpc");
    cfg_clock_request.set_rate(1000.0);
    cfg_clock_request.set_active_edge(nidaqmx_grpc::Edge1::EDGE1_RISING);
    cfg_clock_request.set_sample_mode(nidaqmx_grpc::AcquisitionType::ACQUISITION_TYPE_FINITE_SAMPS);
    cfg_clock_request.set_samps_per_chan(1000);
    auto cfg_clock_response = nidaqmx_grpc::CfgSampClkTimingResponse{};
    client_context = create_context();
    stub_->CfgSampClkTiming(client_context.get(), cfg_clock_request, &cfg_clock_response);
    raise_if_error(stub_, cfg_clock_response);
  }

  void start()
  {
    auto start_task_request = nidaqmx_grpc::StartTaskRequest{};
    start_task_request.mutable_task()->set_name("daq_grpc");
    auto start_task_response = nidaqmx_grpc::StartTaskResponse{};
    auto client_context = create_context();
    stub_->StartTask(client_context.get(), start_task_request, &start_task_response);
    raise_if_error(stub_, start_task_response);

    auto get_num_chans_request = nidaqmx_grpc::GetTaskAttributeUInt32Request{};
    get_num_chans_request.mutable_task()->set_name("daq_grpc");
    get_num_chans_request.set_attribute(nidaqmx_grpc::TaskUInt32Attribute::TASK_ATTRIBUTE_NUM_CHANS);
    auto get_num_chans_response = nidaqmx_grpc::GetTaskAttributeUInt32Response{};
    client_context = create_context();
    stub_->GetTaskAttributeUInt32(client_context.get(), get_num_chans_request, &get_num_chans_response);
    raise_if_error(stub_, get_num_chans_response);

    num_channels_ = get_num_chans_response.value();
  }

  nidaqmx_grpc::ReadAnalogF64Response read()
  {
    auto read_analog_request = nidaqmx_grpc::ReadAnalogF64Request{};
    read_analog_request.mutable_task()->set_name("daq_grpc");
    read_analog_request.set_num_samps_per_chan(1000);
    read_analog_request.set_array_size_in_samps(num_channels_ * 1000);
    read_analog_request.set_fill_mode(nidaqmx_grpc::GroupBy::GROUP_BY_GROUP_BY_CHANNEL);
    read_analog_request.set_timeout(10.0);
    auto read_analog_response = nidaqmx_grpc::ReadAnalogF64Response{};
    auto client_context = create_context();
    stub_->ReadAnalogF64(client_context.get(), read_analog_request, &read_analog_response);
    raise_if_error(stub_, read_analog_response);

    return read_analog_response;
  }

 private:
  StubPtr stub_;
  const std::string physical_channels_;
  google::protobuf::int32 num_channels_{0};
};

void analog_input_demo(const std::string& target_str, const std::string& physical_channels)
{
  auto channel = grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials());

  auto task = DaqTask::create(channel, physical_channels);

  task->configure();

  task->start();

  auto read_analog_response = task->read();

  std::cout << "Read " << read_analog_response.read_array().size() << " Samples." << std::endl;
  std::cout << "First data point: " << read_analog_response.read_array()[0] << std::endl;
}
}  // namespace

// Sample Usage: ./grpc_daq_cpp_demo.exe localhost:31763 Dev1/ai0
int main(int argc, char** argv)
{
  analog_input_demo(argv[1], argv[2]);
}
