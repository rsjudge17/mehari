#include "mehari/CodeGen/Channel.h"

using namespace ChannelDirection;

ChannelP Channel::make_constant(const std::string& constant, unsigned int width) {
  ChannelP ch(new Channel());
  ch->direction = CONSTANT_OUT;
  ch->constant  = constant;
  ch->width     = width;
  return ch;
}
ChannelP Channel::make_port(const std::string& name, ChannelDirection::Direction direction, unsigned int width) {
  ChannelP ch(new Channel());
  ch->direction = direction;
  ch->width     = width;
  ch->data_signal  = name + "_data";
  ch->valid_signal = name + "_valid";
  ch->ready_signal = name + "_ready";
  return ch;
}
ChannelP Channel::make_input(const std::string& name, unsigned int width) {
  return make_port(name, OUT, width);
}
ChannelP Channel::make_output(const std::string& name, unsigned int width) {
  return make_port(name, IN, width);
}
ChannelP Channel::make_component_port(
    ::Operator* component, const std::string& name, ChannelDirection::Direction direction, const OperatorInfo& op_info,
    unsigned int width) {
  ChannelP ch(new Channel());
  ch->direction = direction;
  ch->component = component;
  ch->data_signal  = name + op_info.data_suffix;
  ch->valid_signal = name + op_info.valid_suffix;
  ch->ready_signal = name + op_info.ready_suffix;
  ch->width = (width != 0 ? width : component->getSignalByName(ch->data_signal)->width());
  return ch;
}
ChannelP Channel::make_component_port(
    ::Operator* component, const std::string& name, ChannelDirection::Direction direction, unsigned int width) {
  OperatorInfo op_info;
  op_info.data_suffix = "_data";
  op_info.valid_suffix = "_valid";
  op_info.ready_suffix = "_ready";
  return make_component_port(component, name, direction, op_info, width);
}
ChannelP Channel::make_component_input(::Operator* component, const std::string& name, const OperatorInfo& op_info) {
  return make_component_port(component, name, IN, op_info);
}
ChannelP Channel::make_component_input(::Operator* component, const std::string& name) {
  return make_component_port(component, name, IN);
}
ChannelP Channel::make_component_output(::Operator* component, const std::string& name, const OperatorInfo& op_info) {
  return make_component_port(component, name, OUT, op_info);
}
ChannelP Channel::make_component_output(::Operator* component, const std::string& name) {
  return make_component_port(component, name, OUT);
}
ChannelP Channel::make_component_input_lazy(::Operator* component, const std::string& name, unsigned int width) {
  return make_component_port(component, name, IN, width);
}

ChannelP Channel::make_temp(::Operator* op, const std::string& name, unsigned int width) {
  return make_variable(op, name, width);
}

ChannelP Channel::make_variable(::Operator* op, const std::string& name, unsigned int width) {
  ChannelP ch(new Channel());
  ch->direction = INOUT;
  ch->width     = width;
  ch->data_signal  = op->declare(name + "_data", width, true);
  ch->valid_signal = op->declare(name + "_valid");
  ch->ready_signal = op->declare(name + "_ready");
  return ch;
}

ChannelP Channel::make_part(const std::string& data_channel_part, unsigned int part_width, ChannelP channel) {
  ChannelP ch(new Channel());
  ch->direction = channel->direction;
  ch->width     = part_width;
  ch->data_signal  = channel->data_signal;
  ch->valid_signal = channel->valid_signal;
  ch->ready_signal = channel->ready_signal;
  return ch;
}

void Channel::addTo(::Operator* op) {
  if (direction == IN) {
    op->addOutput(data_signal, width, 1, true);
    op->addOutput(valid_signal);
    op->addInput(ready_signal);
  } else {
    op->addInput(data_signal, width, true);
    op->addInput(valid_signal);
    op->addOutput(ready_signal);
  }
}

void Channel::generateSignal(MyOperator* op, UniqueNameSet& usedVariableNames) {
  if (component && direction != CONSTANT_OUT) {
    //TODO we have to use the instance name instead of the component name
    std::string
      data_signal_new  = usedVariableNames.makeUnique(component->getName() + "_" + data_signal),
      valid_signal_new = usedVariableNames.makeUnique(component->getName() + "_" + valid_signal),
      ready_signal_new = usedVariableNames.makeUnique(component->getName() + "_" + ready_signal);
    if (direction == OUT) {
      op->outPortMap(component, data_signal, data_signal_new);
      op->outPortMap(component, data_signal, data_signal_new);
      op->inPortMap(component, data_signal, data_signal_new);
    } else {
      op->inPortMap(component, data_signal, data_signal_new);
      op->inPortMap(component, data_signal, data_signal_new);
      op->outPortMap(component, data_signal, data_signal_new);
    }

    component = NULL;
    data_signal  = data_signal_new;
    valid_signal = valid_signal_new;
    ready_signal = ready_signal_new;
  }
}

void Channel::outPortMap(MyOperator* op, ::Operator* component, const std::string& port, const std::string& signal,
    UniqueNameSet& usedVariableNames) {
  // The signal already exists, but op->outPortMap(...) insists on creating it,
  // so we use a temporary signal.
  std::string tmp_signal = usedVariableNames.makeUnique(signal + "_");
  op->outPortMap(component, port, tmp_signal);
  *op << "   " << signal << " <= " << tmp_signal << ";\n";
}

void Channel::outPortMapReadySignal(MyOperator* op, ::Operator* component,
    const std::string& port, const std::string& signal, UniqueNameSet& usedVariableNames, ReadySignals& rsignals) {
  std::string tmp_signal = usedVariableNames.makeUnique(signal + "_");
  op->outPortMap(component, port, tmp_signal);

  rsignals.addConsumer(signal, tmp_signal);
}

void Channel::connectToOutput(Channel* ch, MyOperator* op, UniqueNameSet& usedVariableNames, ReadySignals& rsignals) {
  assert(width == ch->width);

  if (direction & IN) {
    if (ch->direction == CONSTANT_OUT) {
      if (!component) {
        *op << "   " << data_signal << " <= " << ch->constant << ";\n";
        *op << "   " << valid_signal << " <= '1';\n";
        // ignore ready_signal
      } else {
        op->inPortMapCst(component, data_signal, ch->constant);
        op->inPortMapCst(component, valid_signal, "'1'");
        // ignore ready_signal
      }
    } else if (ch->direction & OUT) {
      if (component && ch->component) {
        // both are component ports -> generate a signal to connect them
        // We generate the signal for the output port because it can be reused.
        ch->generateSignal(op, usedVariableNames);

        assert(!ch->component);
      }

      if (component && !ch->component) {
        op->inPortMap (component, data_signal,  ch->data_signal);
        op->inPortMap (component, valid_signal, ch->valid_signal);

        outPortMapReadySignal(op, component, ready_signal, ch->ready_signal, usedVariableNames, rsignals);
      } else if (!component && ch->component) {
        outPortMap(op, ch->component, ch->data_signal,  data_signal,  usedVariableNames);
        outPortMap(op, ch->component, ch->valid_signal, valid_signal, usedVariableNames);
        op->inPortMap (ch->component, ch->ready_signal, ready_signal);
      } else if (!component && !ch->component) {
        *op << "   " << data_signal  << " <= " << ch->data_signal << ";\n";
        *op << "   " << valid_signal << " <= " << ch->valid_signal << ";\n";
        rsignals.addConsumer(ch->ready_signal, ready_signal);
      } else {
        assert(false);
      }
    } else {
      throw std::string("Cannot connect input with input!");
    }
  } else {
    if (ch->direction == IN)
      throw std::string("The argument must be an output!");
    else
      throw std::string("Cannot connect output with output!");
  }
}

void Channel::connectToOutput(ChannelP ch, MyOperator* op, UniqueNameSet& usedVariableNames, ReadySignals& rsignals) {
  connectToOutput(ch.get(), op, usedVariableNames, rsignals);
}

void Channel::connectToInput(Channel* ch, MyOperator* op, UniqueNameSet& usedVariableNames, ReadySignals& rsignals) {
  ch->connectToOutput(this, op, usedVariableNames, rsignals);
}

void Channel::connectToInput(ChannelP ch, MyOperator* op, UniqueNameSet& usedVariableNames, ReadySignals& rsignals) {
  connectToInput(ch.get(), op, usedVariableNames, rsignals);
}
