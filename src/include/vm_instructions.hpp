// ========= Internals =========

void vm_next(Program *p);
void _printf(Program *p, const char *format, ...) __attribute__((format(printf, 2, 3)));

byteref _readByte(Program *p)
{
  byteref reference = &p->bytes[p->counter];
  p->counter += 1;

  return reference;
}

Value _readValue(Program *p)
{
  byte type = *(_readByte(p));
  Value value;
  byteref ref;

  switch (type)
  {
  case vt_byte:
  case vt_pin:
  case vt_identifier:
    value.update(type, (void *)_readByte(p));
    break;

  case vt_integer:
  case vt_address:
    ref = &p->bytes[p->counter];
    p->counter += 4;
    value.update(type, (void *)ref);
    break;

  case vt_string:
    // extra \0 at the end of string
    ref = &p->bytes[p->counter];
    p->counter += os_strlen((const char *)ref) + 1;
    value.update(type, (void *)ref);
    break;
  }

  return value;
}

void _printValue(Program *p, Value value)
{
  byte ch;
  switch (value.getType())
  {
  case vt_identifier:
    _printValue(p, p->slots[value.toByte()]);
    return;
  case vt_byte:
    ch = value.toByte();
    if (ch == 0x13 || ch == 0x10)
    {
      _printf(p, "\n");
      return;
    }

    if (ch >= 32 && ch <= 126)
    {
      _printf(p, "%c", value.toByte());
      return;
    }

    _printf(p, "\\x%02x", value.toByte());
    return;

  case vt_pin:
    _printf(p, "%d", value.fromPin());
    return;

  case vt_integer:
    _printf(p, "%d", value.toInteger());
    return;

  case vt_address:
    _printf(p, "%u", value.fromAddress());
    return;

  case vt_string:
    byteref str = value.toString();
    _printf(p, "%s", str);
    return;
  }
}

void _updateSlotWithInteger(Program *p, byte slotId, uint value)
{
  auto type = p->slots[slotId].getType();
  auto valueRef = os_zalloc(sizeof(uint));
  *((uintref)valueRef) = value;

  p->slots[slotId].update(type, valueRef, true);
}

void vm_tick(void *p)
{
  Program *program = (Program *)p;

  if (program->paused)
  {
    os_timer_disarm(&program->timer);
    _printf(program, "\n[!] program is paused\n");
    return;
  }

  while (!program->delayTime && !program->paused)
  {
    vm_next(program);
  }

  if (program->delayTime)
  {
    int delay = program->delayTime;
    program->delayTime = 0;
    os_timer_disarm(&program->timer);
    os_timer_arm(&program->timer, (uint32)delay, 0);
  }
}

void _onInterruptTriggered(void *arg, byte pin)
{
  Program *p = (Program *)arg;
  p->counter = p->interruptHandlers[pin];
  p->paused = false;

  vm_tick(p);
}

// ========= Instructions =========

void MOVE_TO_FLASH vm_binaryOperation(Program *p, byte operation)
{
  auto target = _readValue(p);
  auto a = _readValue(p);
  auto b = _readValue(p);

  auto valueOfA = a.toInteger();
  auto valueOfB = b.toInteger();
  uint newValue;

  switch (operation)
  {

  case op_gt:
    newValue = valueOfA > valueOfB;
    break;
  case op_gte:
    newValue = valueOfA >= valueOfB;
    break;
  case op_lt:
    newValue = valueOfA < valueOfB;
    break;
  case op_lte:
    newValue = valueOfA <= valueOfB;
    break;
  case op_equal:
    newValue = valueOfA == valueOfB;
    break;
  case op_notequal:
    newValue = valueOfA != valueOfB;
    break;
  case op_xor:
    newValue = valueOfA ^ valueOfB;
    break;
  case op_and:
    newValue = valueOfA & valueOfB;
    break;
  case op_or:
    newValue = valueOfA | valueOfB;
    break;
  case op_add:
    newValue = valueOfA + valueOfB;
    break;
  case op_sub:
    newValue = valueOfA - valueOfB;
    break;
  case op_mul:
    newValue = valueOfA * valueOfB;
    break;
  case op_div:
    newValue = valueOfA / valueOfB;
    break;
  case op_mod:
    newValue = valueOfA % valueOfB;
    break;
  }

  _updateSlotWithInteger(p, target.toByte(), newValue);
  _printf(p, "Binary %d: $%d = %d\n", operation, target.toByte(), newValue);
}

void MOVE_TO_FLASH vm_unaryOperation(Program *p, byte operation)
{
  auto target = _readValue(p);
  auto newValue = target.toInteger() + ((operation == op_inc) ? 1 : -1);

  _updateSlotWithInteger(p, target.toByte(), newValue);
  _printf(p, "Unary %d: $%d = %d\n", operation, target.toByte(), newValue);
}

void MOVE_TO_FLASH vm_notOperation(Program *p)
{
  auto target = _readValue(p);
  auto value = !_readValue(p).toBoolean();

  _updateSlotWithInteger(p, target.toByte(), (uint)value);
  _printf(p, "Not %d: %d\n", target.toByte(), value);
}

void MOVE_TO_FLASH vm_assignOperation(Program *p)
{
  auto target = _readValue(p);
  auto value = _readValue(p);

  p->slots[target.toByte()].update(value);
}

void MOVE_TO_FLASH vm_sleep(Program *p)
{
  auto time = _readValue(p).toInteger();

  _printf(p, "sleep %u\n", time);
  os_sleep((uint64)time);
}

void MOVE_TO_FLASH vm_halt(Program *p)
{
  _printf(p, "halt\n");
  p->paused = true;

  if (p->onHalt == nullptr)
  {
    return;
  }

  p->onHalt();
}

void MOVE_TO_FLASH vm_yield(Program *p)
{
  p->delayTime = 1;
}

void MOVE_TO_FLASH vm_delay(Program *p)
{
  p->delayTime = _readValue(p).toInteger();

  if (p->delayTime > MAX_DELAY)
  {
    p->delayTime = MAX_DELAY;
  }

  _printf(p, "delay %d\n", p->delayTime);
}

void MOVE_TO_FLASH vm_ioInterrupt(Program *p)
{
  auto pin = _readValue(p).toByte();
  auto mode = _readValue(p).toByte();
  auto position = _readValue(p).toInteger();
  void *handler = (void *)&_onInterruptTriggered;

  p->interruptHandlers[pin] = position;
  _printf(p, "interrupt pin %d, mode %d, jump to %d\n", pin, mode, position);
  os_io_interrupt(pin, handler, (void *)p, mode);
}

void MOVE_TO_FLASH vm_ioInterruptToggle(Program *p)
{
  auto enabled = _readValue(p).toBoolean();

  if (enabled)
  {
    os_io_enableInterrupts();
    _printf(p, "interrupts armed\n");
    return;
  }

  os_io_disableInterrupts();
  _printf(p, "interrupts disarmed\n");
}

void MOVE_TO_FLASH vm_jumpTo(Program *p)
{
  auto position = _readValue(p);
  p->counter = position.toInteger();
  _printf(p, "jump to %d\n", p->counter);
}

void MOVE_TO_FLASH vm_jumpIf(Program *p)
{
  auto condition = _readValue(p);
  auto position = _readValue(p);

  if (!condition.toBoolean())
    return;

  p->counter = position.toInteger();
  _printf(p, "jump if: %d\n", p->counter);
}

void MOVE_TO_FLASH vm_toggleDebug(Program *p)
{
  auto value = _readValue(p).toBoolean();

  if (value)
  {
    os_enableSerial();
    _printf(p, "serial debug on\n");
    return;
  }

  _printf(p, "serial debug off\n");
  os_disableSerial();
}

void MOVE_TO_FLASH vm_printStationStatus(Program *p)
{
  // TODO
}

void MOVE_TO_FLASH vm_systemInformation(Program *p)
{
  _printf(p, "Time now: %u\n", os_time() / 1000);
  _printf(p, "Free mem: %u bytes\n", os_freeHeapSize());
}

void MOVE_TO_FLASH vm_dump(Program *p)
{
  uint i = 0;
  _printf(p, "\nProgram\n");
  while (i < p->endOfTheProgram)
  {
    _printf(p, "%02x ", p->bytes[i++]);
  }

  _printf(p, "\nSlots\n");
  for (i = 0; i < MAX_SLOTS; i++)
  {
    _printf(p, "%d: '", i);
    _printValue(p, p->slots[i]);
    _printf(p, "'\n");
  }

  _printf(p, "\nInterrupts\n");
  for (i = 0; i < NUMBER_OF_PINS; i++)
  {
    _printf(p, "%d: %d\n", i, p->interruptHandlers[i]);
  }
}

void MOVE_TO_FLASH vm_declareReference(Program *p)
{
  auto slotId = _readValue(p).toByte();
  auto value = _readValue(p);

  p->slots[slotId].update(value);

  _printf(p, "declare %d, %d = ", slotId, p->slots[slotId].getType());
  _printValue(p, p->slots[slotId]);
  _printf(p, "\n");
}

void MOVE_TO_FLASH vm_readFromMemory(Program *p)
{
  auto slotId = _readValue(p).toByte();
  auto address = _readValue(p).toInteger();

  _printf(p, "memget [%d], %u\n", slotId, address);

  // if (1)
  // {
  //   if (address < 0x3ff00000 || address >= 0x60010000)
  //   {
  //     return;
  //   }

  //   uint32_t val_aligned = *(uint32_t *)(addr & (~3));
  //   uint32_t shift = (addr & 3) * 8;
  //   return (val_aligned >> shift) & 0xff;
  // }

  p->slots[slotId].update(vt_address, (void *)address);
}

void MOVE_TO_FLASH vm_writeToMemory(Program *p)
{
  auto address = (void *)_readValue(p).toInteger();
  auto value = _readValue(p);

  _printf(p, "memset %p\n", address);

  switch (value.getType())
  {
  case vt_byte:
    os_mem_write(address, value.toByte());
    break;

  case vt_pin:
    os_mem_write(address, value.fromPin());
    break;

  case vt_integer:
    os_mem_write(address, value.toInteger());
    break;

  case vt_address:
    os_mem_write(address, os_mem_read(value.toInteger()));
    break;
  }
}

void MOVE_TO_FLASH vm_ioMode(Program *p)
{
  auto pin = _readValue(p).toByte();
  auto value = _readValue(p).toByte();

  _printf(p, "io mode %d %d\n", pin, value);

  if (value >= 0 && value <= 3)
  {
    os_io_mode(pin, value);
  }
}

void MOVE_TO_FLASH vm_ioType(Program *p)
{
  auto pin = _readValue(p).toByte();
  auto value = _readValue(p).toByte();

  _printf(p, "io type %d %d\n", pin, value);
  if (value >= 0 && value <= 4)
  {
    os_io_type(pin, value);
  }
}

void MOVE_TO_FLASH vm_ioWrite(Program *p)
{
  auto pin = _readValue(p).toByte();
  auto value = _readValue(p).toBoolean();

  _printf(p, "io write %d %d\n", pin, value);
  os_io_write(pin, (bool)value);
}

void MOVE_TO_FLASH vm_ioRead(Program *p)
{
  auto target = _readValue(p);
  auto pin = _readValue(p).toByte();
  auto pinValue = (byte)os_io_read(pin);

  _updateSlotWithInteger(p, target.toByte(), (uint)pinValue);
  _printf(p, "io read %d, %d\n", pin, pinValue);
}

void MOVE_TO_FLASH vm_ioAllOut(Program *p)
{
  _printf(p, "io all out\n");
  os_io_allOutput();
}

void MOVE_TO_FLASH vm_startAccessPoint(Program *p)
{
  _printf(p, "startAccessPoint\n");
  os_wifi_ap();
}

void MOVE_TO_FLASH vm_wifiConnect(Program *p)
{
  auto ssid = _readValue(p).toString();
  auto password = _readValue(p);
  bool hasPassword = password.getType() == vt_string;

  _printf(p, "connect to %s / %s\n", ssid, password.toString());

  if (hasPassword)
  {
    os_wifi_connect((const char *)ssid, (const char *)password.toString());
    return;
  }

  os_wifi_connect((const char *)ssid, (const char *)nullptr);
}

void MOVE_TO_FLASH vm_wifiDisconnect(Program *p)
{
  os_wifi_disconnect();
}

void MOVE_TO_FLASH vm_wifiList(Program *p)
{
  // TODO
}

void MOVE_TO_FLASH vm_i2csetup(Program *p)
{
  auto dataPin = _readValue(p).toByte();
  auto clockPin = _readValue(p).toByte();

  os_i2c_setup(dataPin, clockPin);
  _printf(p, "i2c setup SDA %d, SCK %d\n", dataPin, clockPin);
}

void MOVE_TO_FLASH vm_i2cstart(Program *p)
{
  _printf(p, "i2c start\n");
  os_i2c_start();
}

void MOVE_TO_FLASH vm_i2cstop(Program *p)
{
  _printf(p, "i2c stop\n");
  os_i2c_stop();
}

void MOVE_TO_FLASH vm_i2cwrite(Program *p)
{
  auto byte = _readValue(p).toByte();
  _printf(p, "i2c write %d\n", byte);
  os_i2c_writeByteAndAck(byte);
}

void MOVE_TO_FLASH vm_i2cfind(Program *p)
{
  auto slotId = _readValue(p).toByte();
  byte deviceId = os_i2c_findDevice();
  byte *value = (byte *)os_zalloc(sizeof(byte));
  *value = deviceId;
  p->slots[slotId].update(vt_byte, value, true);
  _printf(p, "i2c find %d\n", slotId);
}

void MOVE_TO_FLASH vm_i2cread(Program *p)
{
  auto target = _readValue(p);
  byte value = os_i2c_readByte();
  void *v = os_zalloc(sizeof(byte));
  *(byte *)v = value;
  p->slots[target.toByte()].update(vt_byte, v, true);
  _printf(p, "i2cread %d\n", value);
}

void _printf(Program *p, const char *format, ...)
{
  if (p->onSend == nullptr)
  {
    return;
  }

  va_list arg;
  va_start(arg, format);
  char buffer[1024];
  int length = os_sprintf(buffer, format, arg);
  va_end(arg);
  p->onSend(buffer, length);
}

void MOVE_TO_FLASH vm_load(Program *program, byteref _bytes, int length)
{
  if (program->bytes != nullptr && program->endOfTheProgram < length)
  {
    program->bytes = (byteref)os_realloc(program->bytes, length);
  }

  if (program->bytes == nullptr)
  {
    program->bytes = (byteref)os_zalloc(length);
  }

  os_memcpy(program->bytes, _bytes, length);
  program->endOfTheProgram = length;
  program->counter = 0;
  program->paused = false;

  _printf(program, "[i] Loaded %d bytes\n", length);
  os_timer_setfn(&program->timer, &vm_tick, program);
  os_timer_arm(&program->timer, 1, 0);
}

void vm_next(Program *p)
{
  byte next = *(_readByte(p));

  switch (next)
  {
  case op_noop:
    break;

  case op_halt:
    vm_halt(p);
    break;

  case op_restart:
    os_restart();
    break;

  case op_debug:
    vm_toggleDebug(p);
    break;

  case op_systeminfo:
    vm_systemInformation(p);
    break;

  case op_sleep:
    vm_sleep(p);
    break;

  case op_print:
    _printValue(p, _readValue(p));
    break;

  case op_dump:
    vm_dump(p);
    break;

  case op_declare:
    vm_declareReference(p);
    break;

  case op_memget:
    vm_readFromMemory(p);
    break;

  case op_memset:
    vm_writeToMemory(p);
    break;

  case op_iowrite:
    vm_ioWrite(p);
    break;

  case op_ioread:
    vm_ioRead(p);
    break;

  case op_iomode:
    vm_ioMode(p);
    break;

  case op_iotype:
    vm_ioType(p);
    break;

  case op_ioallout:
    vm_ioAllOut(p);
    break;

  case op_iointerrupt:
    vm_ioInterrupt(p);
    break;

  case op_iointerruptToggle:
    vm_ioInterruptToggle(p);
    break;

  case op_delay:
    vm_delay(p);
    break;

  case op_yield:
    vm_yield(p);
    break;

  case op_jumpto:
    vm_jumpTo(p);
    break;

  case op_jumpif:
    vm_jumpIf(p);
    break;

  case op_gt:
  case op_gte:
  case op_lt:
  case op_lte:
  case op_equal:
  case op_notequal:
  case op_xor:
  case op_and:
  case op_or:
  case op_add:
  case op_sub:
  case op_mul:
  case op_div:
  case op_mod:
    vm_binaryOperation(p, next);
    break;

  case op_inc:
  case op_dec:
    vm_unaryOperation(p, next);
    break;

  case op_not:
    vm_notOperation(p);
    break;

  case op_assign:
    vm_assignOperation(p);
    break;

  case op_wifistatus:
    vm_printStationStatus(p);
    break;

  case op_wifiap:
    vm_startAccessPoint(p);
    break;

  case op_wificonnect:
    vm_wifiConnect(p);
    break;

  case op_wifidisconnect:
    vm_wifiDisconnect(p);
    break;

  case op_wifilist:
    vm_wifiList(p);
    break;

  case op_i2csetup:
    vm_i2csetup(p);
    break;
  case op_i2cstart:
    vm_i2cstart(p);
    break;
  case op_i2cstop:
    vm_i2cstop(p);
    break;
  case op_i2cwrite:
    vm_i2cwrite(p);
    break;
  case op_i2cread:
    vm_i2cread(p);
    break;
  // case op_i2csetack:
  //   vm_i2c_setack(p);
  //   break;
  // case op_i2cgetack:
  //   vm_i2c_getack(p);
  //   break;
  case op_i2cfind:
    vm_i2cfind(p);
    break;

  default:
    _printf(p, "\n[!] Invalid operation: %02x\n", next);
    vm_halt(p);
  }

  if (p->counter >= p->endOfTheProgram)
  {
    vm_halt(p);
  }
}
