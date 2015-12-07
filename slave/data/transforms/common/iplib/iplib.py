"""
iplib module.

The representation of IPv4 addresses and netmasks.
You can use this module to convert amongst many different notations
and to manage couples of address/netmask in the CIDR notation.

  Copyright 2001-2008 Davide Alberani <da@erlug.linux.it>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""

__version__ = '1.1'

from types import IntType, LongType

_IntegerTypes = (IntType, LongType)


# Notation types (with an example in the comment).
# You can use these constants when you have to specify a notation style.
IP_UNKNOWN = NM_UNKNOWN = 0
IP_DOT = NM_DOT = 1 # 192.168.0.42
IP_HEX = NM_HEX = 2 # 0xC0A8002A
IP_BIN = NM_BIN = 3 # 030052000052
IP_OCT = NM_OCT = 4 # 11000000101010000000000000101010
IP_DEC = NM_DEC = 5 # 3232235562
NM_BITS = 6 # 26
NM_WILDCARD = 7 # 0.0.0.63

# Map notations with one or more strings.
# You can use these constant strings when you have to specify a notation
# style, instead of using numeric values.
NOTATION_MAP = {
    IP_DOT:         ('dotted decimal', 'dotted', 'quad', 'dot', 'dotted quad'),
    IP_HEX:         ('hexadecimal', 'hex'),
    IP_BIN:         ('binary', 'bin'),
    IP_OCT:         ('octal', 'oct'),
    IP_DEC:         ('decimal', 'dec'),
    NM_BITS:        ('bits', 'bit', 'cidr'),
    NM_WILDCARD:    ('wildcard bits', 'wildcard'),
    IP_UNKNOWN:     ('unknown', 'unk')
}
_NOTATION_KEYS = dict([(key, key) for key in NOTATION_MAP.keys()])
for key, values in NOTATION_MAP.items():
    for value in values:
        _NOTATION_KEYS[value] = key

def _get_notation(notation):
    """Given a numeric value or string value, returns one in IP_DOT, IP_HEX,
    IP_BIN, etc., or None if unable to convert to the internally
    used numeric convention."""
    return _NOTATION_KEYS.get(notation, None)

def p_notation(notation):
    """Return a string representing the given notation."""
    return NOTATION_MAP[_get_notation(notation) or IP_UNKNOWN][0]

# This dictionary maps NM_BITS to NM_DEC values. 
# NOTE: /31 is a valid netmask; see RFC3021 (courtesy of Lars Erik Gullerud).
VALID_NETMASKS = {0: 0L, 1: 2147483648L, 2: 3221225472L, 3: 3758096384L,
                    4: 4026531840L, 5: 4160749568L, 6: 4227858432L,
                    7: 4261412864L, 8: 4278190080L, 9: 4286578688L,
                    10: 4290772992L, 11: 4292870144L, 12: 4293918720L,
                    13: 4294443008L, 14: 4294705152L, 15: 4294836224L,
                    16: 4294901760L, 17: 4294934528L, 18: 4294950912L,
                    19: 4294959104L, 20: 4294963200L, 21: 4294965248L,
                    22: 4294966272L, 23: 4294966784L, 24: 4294967040L,
                    25: 4294967168L, 26: 4294967232L, 27: 4294967264L,
                    28: 4294967280L, 29: 4294967288L, 30: 4294967292L,
                    31: 4294967294L, 32: 4294967295L}
_NETMASKS_VALUES = VALID_NETMASKS.values()
_NETMASKS_INV = dict([(value, key) for key, value in VALID_NETMASKS.items()])


# - Functions used to check if an address or a netmask is in a given notation.

def is_dot(ip):
    """Return true if the IP address is in dotted decimal notation."""
    octets = str(ip).split('.')
    if len(octets) != 4:
        return False
    for i in octets:
        try:
            val = long(i)
        except ValueError:
            return False
        if val > 255 or val < 0:
            return False
    return True

def is_hex(ip):
    """Return true if the IP address is in hexadecimal notation."""
    try:
        dec = long(str(ip), 16)
    except (TypeError, ValueError):
        return False
    if dec > 0xFFFFFFFFL or dec < 0:
        return False
    return True

def is_bin(ip):
    """Return true if the IP address is in binary notation."""
    try:
        ip = str(ip)
        if len(ip) != 32:
            return False
        dec = long(ip, 2)
    except (TypeError, ValueError):
        return False
    if dec > 4294967295L or dec < 0:
        return False
    return True

def is_oct(ip):
    """Return true if the IP address is in octal notation."""
    try:
        dec = long(str(ip), 8)
    except (TypeError, ValueError):
        return False
    if dec > 037777777777L or dec < 0:
        return False
    return True

def is_dec(ip):
    """Return true if the IP address is in decimal notation."""
    try:
        dec = long(str(ip))
    except ValueError:
        return False
    if dec > 4294967295L or dec < 0:
        return False
    return True


def _check_nm(nm, notation):
    """Function internally used to check if the given netmask
    is of the specified notation."""
    # Convert to decimal, and check if it's in the list of valid netmasks.
    _NM_CHECK_FUNCT = {
        NM_DOT: _dot_to_dec,
        NM_HEX: _hex_to_dec,
        NM_BIN: _bin_to_dec,
        NM_OCT: _oct_to_dec,
        NM_DEC: _dec_to_dec_long}
    try:
        dec = _NM_CHECK_FUNCT[notation](nm, check=True)
    except ValueError:
        return False
    if dec in _NETMASKS_VALUES:
        return True
    return False

def is_dot_nm(nm):
    """Return true if the netmask is in dotted decimal notatation."""
    return _check_nm(nm, NM_DOT)

def is_hex_nm(nm):
    """Return true if the netmask is in hexadecimal notatation."""
    return _check_nm(nm, NM_HEX)

def is_bin_nm(nm):
    """Return true if the netmask is in binary notatation."""
    return _check_nm(nm, NM_BIN)

def is_oct_nm(nm):
    """Return true if the netmask is in octal notatation."""
    return _check_nm(nm, NM_OCT)

def is_dec_nm(nm):
    """Return true if the netmask is in decimal notatation."""
    return _check_nm(nm, NM_DEC)

def is_bits_nm(nm):
    """Return true if the netmask is in bits notatation."""
    try:
        bits = long(str(nm))
    except ValueError:
        return False
    if bits > 32 or bits < 0:
        return False
    return True

def is_wildcard_nm(nm):
    """Return true if the netmask is in wildcard bits notatation."""
    try:
        dec = 0xFFFFFFFFL - _dot_to_dec(nm, check=True)
    except ValueError:
        return False
    if dec in _NETMASKS_VALUES:
        return True
    return False


# - Functions used to convert various notation to/from decimal notation.

def _dot_to_dec(ip, check=True):
    """Dotted decimal notation to decimal conversion."""
    if check and not is_dot(ip):
        raise ValueError, '_dot_to_dec: invalid IP: "%s"' % ip
    octets = str(ip).split('.')
    dec = 0L
    dec |= long(octets[0]) << 24
    dec |= long(octets[1]) << 16
    dec |= long(octets[2]) << 8
    dec |= long(octets[3])
    return dec

def _dec_to_dot(ip):
    """Decimal to dotted decimal notation conversion."""
    first = int((ip >> 24) & 255)
    second = int((ip >> 16) & 255)
    third = int((ip >> 8) & 255)
    fourth = int(ip & 255)
    return '%d.%d.%d.%d' % (first, second, third, fourth)


def _hex_to_dec(ip, check=True):
    """Hexadecimal to decimal conversion."""
    if check and not is_hex(ip):
        raise ValueError, '_hex_to_dec: invalid IP: "%s"' % ip
    if isinstance(ip, _IntegerTypes):
        ip = hex(ip)
    return long(str(ip), 16)

def _dec_to_hex(ip):
    """Decimal to hexadecimal conversion."""
    return hex(ip)[:-1]


def _oct_to_dec(ip, check=True):
    """Octal to decimal conversion."""
    if check and not is_oct(ip):
        raise ValueError, '_oct_to_dec: invalid IP: "%s"' % ip
    if isinstance(ip, _IntegerTypes):
        ip = oct(ip)
    return long(str(ip), 8)

def _dec_to_oct(ip):
    """Decimal to octal conversion."""
    return oct(ip)[:-1]


def _bin_to_dec(ip, check=True):
    """Binary to decimal conversion."""
    if check and not is_bin(ip):
        raise ValueError, '_bin_to_dec: invalid IP: "%s"' % ip
    if isinstance(ip, _IntegerTypes):
        ip = str(ip)
    return long(str(ip), 2)

def _BYTES_TO_BITS():
    """Generate a table to convert a whole byte to binary.
    This code was taken from the Python Cookbook, 2nd edition - O'Reilly."""
    the_table = 256*[None]
    bits_per_byte = range(7, -1, -1)
    for n in xrange(256):
        l = n
        bits = 8*[None]
        for i in bits_per_byte:
            bits[i] = '01'[n & 1]
            n >>= 1
        the_table[l] = ''.join(bits)
    return the_table
_BYTES_TO_BITS = _BYTES_TO_BITS()

def _dec_to_bin(ip):
    """Decimal to binary conversion."""
    bits = []
    while ip:
        bits.append(_BYTES_TO_BITS[ip & 255])
        ip >>= 8
    bits.reverse()
    return ''.join(bits) or 32*'0'


def _dec_to_dec_long(ip, check=True):
    """Decimal to decimal (long) conversion."""
    if check and not is_dec(ip):
        raise ValueError, '_dec_to_dec: invalid IP: "%s"' % ip
    return long(str(ip))

def _dec_to_dec_str(ip):
    """Decimal to decimal (string) conversion."""
    return str(ip)


def _bits_to_dec(nm, check=True):
    """Bits to decimal conversion."""
    if check and not is_bits_nm(nm):
        raise ValueError, '_bits_to_dec: invalid netmask: "%s"' % nm
    bits = long(str(nm))
    return VALID_NETMASKS[bits]

def _dec_to_bits(nm):
    """Decimal to bits conversion."""
    return str(_NETMASKS_INV[nm])


def _wildcard_to_dec(nm, check=False):
    """Wildcard bits to decimal conversion."""
    if check and not is_wildcard_nm(nm):
        raise ValueError, '_wildcard_to_dec: invalid netmask: "%s"' % nm
    return 0xFFFFFFFFL - _dot_to_dec(nm, check=False)

def _dec_to_wildcard(nm):
    """Decimal to wildcard bits conversion."""
    return _dec_to_dot(0xFFFFFFFFL - nm)

# - Functions used to detect the notation of an IP address or netmask.

_CHECK_FUNCT = {
    IP_DOT: (is_dot, is_dot_nm),
    IP_HEX: (is_hex, is_hex_nm),
    IP_BIN: (is_bin, is_bin_nm),
    IP_OCT: (is_oct, is_oct_nm),
    IP_DEC: (is_dec, is_dec_nm),
    NM_BITS: (lambda: False, is_bits_nm),
    NM_WILDCARD: (lambda: False, is_wildcard_nm)
}
_CHECK_FUNCT_KEYS = _CHECK_FUNCT.keys()

def _is_notation(ip, notation, _isnm):
    """Internally used to check if an IP/netmask is in the given notation."""
    notation_orig = notation
    notation = _get_notation(notation)
    if notation not in _CHECK_FUNCT_KEYS:
        raise ValueError, '_is_notation: unkown notation: "%s"' % notation_orig
    return _CHECK_FUNCT[notation][_isnm](ip)

def is_notation(ip, notation):
    """Return true if the given address is in the given notation."""
    return _is_notation(ip, notation, _isnm=False)

def is_notation_nm(nm, notation):
    """Return true if the given netmask is in the given notation."""
    return _is_notation(nm, notation, _isnm=True)


def _detect(ip, _isnm):
    """Function internally used to detect the notation of the
    given IP or netmask."""
    ip = str(ip)
    if len(ip) > 1:
        if ip[0:2] == '0x':
            if _CHECK_FUNCT[IP_HEX][_isnm](ip):
                return IP_HEX
        elif ip[0] == '0':
            if _CHECK_FUNCT[IP_OCT][_isnm](ip):
                return IP_OCT
    if _CHECK_FUNCT[IP_DOT][_isnm](ip):
        return IP_DOT
    elif _isnm and _CHECK_FUNCT[NM_BITS][_isnm](ip):
        return NM_BITS
    elif _CHECK_FUNCT[IP_DEC][_isnm](ip):
        return IP_DEC
    elif _isnm and _CHECK_FUNCT[NM_WILDCARD][_isnm](ip):
        return NM_WILDCARD
    elif _CHECK_FUNCT[IP_BIN][_isnm](ip):
        return IP_BIN
    return IP_UNKNOWN

def detect(ip):
    """Detect the notation of an IP address.

    @param ip: the IP address.
    @type ip: integers, strings or object with an appropriate __str()__ method.
    @return: one of the IP_* constants; IP_UNKNOWN if undetected."""
    return _detect(ip, _isnm=False)

def detect_nm(nm):
    """Detect the notation of a netmask.
    @param nm: the netmask.
    @type nm: integers, strings or object with an appropriate __str()__ method.
    @return: one of the NM_* constants; NM_UNKNOWN if undetected."""
    return _detect(nm, _isnm=True)

def p_detect(ip):
    """Return the notation of an IP address (string)."""
    return NOTATION_MAP[detect(ip)][0]

def p_detect_nm(nm):
    """Return the notation of a netmask (string)."""
    return NOTATION_MAP[detect_nm(nm)][0]


def _convert(ip, notation, inotation, _check, _isnm):
    """Internally used to convert IPs and netmasks to other notations."""
    inotation_orig = inotation
    notation_orig = notation
    inotation = _get_notation(inotation)
    notation = _get_notation(notation)
    if inotation is None:
        raise ValueError, '_convert: unknown input notation: "%s"' % \
                                                                inotation_orig
    if notation is None:
        raise ValueError, '_convert: unknown output notation: "%s"' % \
                                                                notation_orig
    docheck = _check or False
    if inotation == IP_UNKNOWN:
        inotation = _detect(ip, _isnm)
        if inotation == IP_UNKNOWN:
            raise ValueError, \
                    '_convert: unable to guess input notation or invalid value'
        if _check is None:
            docheck = True
    # We _always_ check this case later.
    if _isnm:
        docheck = False
    dec = 0L
    if inotation == IP_DOT:
        dec = _dot_to_dec(ip, docheck)
    elif inotation == IP_HEX:
        dec = _hex_to_dec(ip, docheck)
    elif inotation == IP_BIN:
        dec = _bin_to_dec(ip, docheck)
    elif inotation == IP_OCT:
        dec = _oct_to_dec(ip, docheck)
    elif inotation == IP_DEC:
        dec = _dec_to_dec_long(ip, docheck)
    elif _isnm and inotation == NM_BITS:
        dec = _bits_to_dec(ip, docheck)
    elif _isnm and inotation == NM_WILDCARD:
        dec = _wildcard_to_dec(ip, docheck)
    else:
        raise ValueError, '_convert: unknown IP/netmask notation: "%s"' % \
                                                                inotation_orig
    # Ensure this is a valid netmask.
    if _isnm and dec not in _NETMASKS_VALUES:
        raise ValueError, '_convert: invalid netmask: "%s"' % ip
    if notation == IP_DOT:
        return _dec_to_dot(dec)
    elif notation == IP_HEX:
        return _dec_to_hex(dec)
    elif notation == IP_BIN:
        return _dec_to_bin(dec)
    elif notation == IP_OCT:
        return _dec_to_oct(dec)
    elif notation == IP_DEC:
        return _dec_to_dec_str(dec)
    elif _isnm and notation == NM_BITS:
        return _dec_to_bits(dec)
    elif _isnm and notation == NM_WILDCARD:
        return _dec_to_wildcard(dec)
    else:
        raise ValueError, 'convert: unknown notation: "%s"' % notation_orig

def convert(ip, notation=IP_DOT, inotation=IP_UNKNOWN, check=True):
    """Convert among IP address notations.

    Given an IP address, this function returns the address
    in another notation.

    @param ip: the IP address.
    @type ip: integers, strings or object with an appropriate __str()__ method.

    @param notation: the notation of the output (default: IP_DOT).
    @type notation: one of the IP_* constants, or the equivalent strings.

    @param inotation: force the input to be considered in the given notation
                    (default the notation of the input is autodetected).
    @type inotation: one of the IP_* constants, or the equivalent strings.

    @param check: force the notation check on the input.
    @type check: True force the check, False force not to check and None
                do the check only if the inotation is unknown.

    @return: a string representing the IP in the selected notation.

    @raise ValueError: raised when the input is in unknown notation."""
    return _convert(ip, notation, inotation, _check=check, _isnm=False)

def convert_nm(nm, notation=IP_DOT, inotation=IP_UNKNOWN, check=True):
    """Convert a netmask to another notation."""
    return _convert(nm, notation, inotation, _check=check, _isnm=True)


# - Classes used to manage IP addresses, netmasks and the CIDR notation.

class _IPv4Base(object):
    """Base class for IP addresses and netmasks."""
    _isnm = False # Set to True when representing a netmask.

    def __init__(self, ip, notation=IP_UNKNOWN):
        """Initialize the object."""
        self.set(ip, notation)

    def set(self, ip, notation=IP_UNKNOWN):
        """Set the IP address/netmask."""
        self._ip_dec = long(_convert(ip, notation=IP_DEC, inotation=notation,
                            _check=True, _isnm=self._isnm))
        self._ip = _convert(self._ip_dec, notation=IP_DOT, inotation=IP_DEC,
                            _check=False, _isnm=self._isnm)

    def get(self):
        """Return the address/netmask."""
        return self.get_dot()

    def get_dot(self):
        """Return the dotted decimal notation of the address/netmask."""
        return self._ip

    def get_hex(self):
        """Return the hexadecimal notation of the address/netmask."""
        return _convert(self._ip_dec, notation=IP_HEX,
                        inotation=IP_DEC, _check=False, _isnm=self._isnm)

    def get_bin(self):
        """Return the binary notation of the address/netmask."""
        return _convert(self._ip_dec, notation=IP_BIN,
                        inotation=IP_DEC, _check=False, _isnm=self._isnm)

    def get_dec(self):
        """Return the decimal notation of the address/netmask."""
        return str(self._ip_dec)

    def get_oct(self):
        """Return the octal notation of the address/netmask."""
        return _convert(self._ip_dec, notation=IP_OCT,
                        inotation=IP_DEC, _check=False, _isnm=self._isnm)

    def __str__(self):
        """Print this address/netmask."""
        return self.get()

    def _cmp_prepare(self, other):
        """Prepare the item to be compared with this address/netmask."""
        if isinstance(other, self.__class__):
            return other._ip_dec
        elif isinstance(other, _IntegerTypes):
            # NOTE: this hides the fact that "other" can be a non valid IP/nm.
            return other
        return self.__class__(other)._ip_dec

    def __cmp__(self, other):
        """Compare two addresses/netmasks."""
        cmp_val = 0
        if self._ip_dec < self._cmp_prepare(other):
            cmp_val = -1
        elif self._ip_dec > self._cmp_prepare(other):
            cmp_val = 1
        if self._isnm:
            # NOTE: for netmasks invert values; compare netmasks by width.
            if cmp_val == -1: cmp_val = 1
            elif cmp_val == 1: cmp_val = -1
        return cmp_val

    def __int__(self):
        """Return the decimal representation of the address/netmask."""
        return self._ip_dec

    def __long__(self):
        """Return the decimal representation of the address/netmask (long)."""
        return long(self._ip_dec)

    def __hex__(self):
        """Return the hexadecimal representation of the address/netmask."""
        return self.get_hex()

    def __oct__(self):
        """Return the octal representation of the address/netmask."""
        return self.get_oct()

    if not _isnm: ip = address = property(get, set, doc='The represented IP.')
    else: nm = netmask = property(get, set, doc='The represented netmask.')


class IPv4Address(_IPv4Base):
    """An IPv4 Internet address.

    This class represents an IPv4 Internet address."""

    def __repr__(self):
        """The representation string for this address."""
        return '<IPv4 address %s>' % self.get()

    def _add(self, other):
        """Sum two IP addresses."""
        if isinstance(other, self.__class__):
            sum = self._ip_dec + other._ip_dec
        elif isinstance(other, _IntegerTypes):
            sum = self._ip_dec + other
        else:
            other = self.__class__(other)
            sum = self._ip_dec + other._ip_dec
        return sum

    def __add__(self, other):
        """Sum two IP addresses."""
        return IPv4Address(self._add(other), notation=IP_DEC)

    __radd__ = __add__

    def __iadd__(self, other):
        """Augmented arithmetic sum."""
        self.set(self._add(other), notation=IP_DEC)
        return self

    def _sub(self, other):
        """Subtract two IP addresses."""
        if isinstance(other, self.__class__):
            sub = self._ip_dec - other._ip_dec
        if isinstance(other, _IntegerTypes):
            sub = self._ip_dec - other
        else:
            other = self.__class__(other)
            sub = self._ip_dec - other._ip_dec
        return sub

    def __sub__(self, other):
        """Subtract two IP addresses."""
        return IPv4Address(self._sub(other), notation=IP_DEC)

    __rsub__ = __sub__

    def __isub__(self, other):
        """Augmented arithmetic subtraction."""
        self.set(self._sub(other), notation=IP_DEC)
        return self


class IPv4NetMask(_IPv4Base):
    """An IPv4 Internet netmask.

    This class represents an IPv4 Internet netmask."""
    _isnm = True

    def get_bits(self):
        """Return the bits notation of the netmask."""
        return _convert(self._ip, notation=NM_BITS,
                        inotation=IP_DOT, _check=False, _isnm=self._isnm)

    def get_wildcard(self):
        """Return the wildcard bits notation of the netmask."""
        return _convert(self._ip, notation=NM_WILDCARD,
                        inotation=IP_DOT, _check=False, _isnm=self._isnm)

    def __repr__(self):
        """The representation string for this netmask."""
        return '<IPv4 netmask %s>' % self.get()


class CIDR(object):
    """A CIDR address.

    The representation of a Classless Inter-Domain Routing (CIDR) address."""
    def __init__(self, ip, netmask=None):
        self.set(ip, netmask)

    def set(self, ip, netmask=None):
        """Set the IP address and the netmask."""
        if isinstance(ip, basestring) and netmask is None:
            ipnm = ip.split('/')
            if len(ipnm) != 2:
                raise ValueError, 'set: invalid CIDR: "%s"' % ip
            ip = ipnm[0]
            netmask = ipnm[1]
        if isinstance(ip, IPv4Address):
            self._ip = ip
        else:
            self._ip = IPv4Address(ip)
        if isinstance(netmask, IPv4NetMask):
            self._nm = netmask
        else:
            self._nm = IPv4NetMask(netmask)
        ipl = long(self._ip)
        nml = long(self._nm)
        base_add = ipl & nml
        self._ip_num = 0xFFFFFFFFL - 1 - nml
        # NOTE: quite a mess.
        #      This's here to handle /32 (-1) and /31 (0) netmasks.
        if self._ip_num in (-1, 0):
            if self._ip_num == -1: self._ip_num = 1
            else: self._ip_num = 2
            self._net_ip = None
            self._bc_ip = None
            self._first_ip_dec = base_add
            self._first_ip = IPv4Address(self._first_ip_dec, notation=IP_DEC)
            if self._ip_num == 1: last_ip_dec = self._first_ip_dec
            else: last_ip_dec = self._first_ip_dec + 1
            self._last_ip = IPv4Address(last_ip_dec, notation=IP_DEC)
            return None
        self._net_ip = IPv4Address(base_add, notation=IP_DEC)
        self._bc_ip = IPv4Address(base_add + self._ip_num + 1, notation=IP_DEC)
        self._first_ip_dec = base_add + 1
        self._first_ip = IPv4Address(self._first_ip_dec, notation=IP_DEC)
        self._last_ip = IPv4Address(base_add + self._ip_num, notation=IP_DEC)

    def get(self):
        """Print this CIDR address."""
        return '%s/%s' % (str(self._ip), str(self._nm))

    def set_ip(self, ip):
        """Change the current IP."""
        self.set(ip=ip, netmask=self._nm)

    def get_ip(self):
        """Return the given address."""
        return self._ip

    def set_netmask(self, netmask):
        """Change the current netmask."""
        self.set(ip=self._ip, netmask=netmask)

    def get_netmask(self):
        """Return the netmask."""
        return self._nm

    def get_first_ip(self):
        """Return the first usable IP address."""
        return self._first_ip

    def get_last_ip(self):
        """Return the last usable IP address."""
        return self._last_ip

    def get_network_ip(self):
        """Return the network address."""
        return self._net_ip

    def get_broadcast_ip(self):
        """Return the broadcast address."""
        return self._bc_ip

    def get_ip_number(self):
        """Return the number of usable IP addresses."""
        return self._ip_num

    def get_all_valid_ip(self):
        """Return a list of IPv4Address objects, one for every usable IP.

        WARNING: it's slow and can take a huge amount of memory for
        subnets with a large number of addresses.
        Use __iter__ instead ('for ip in ...')."""
        return list(self.__iter__())

    def is_valid_ip(self, ip):
        """Return true if the given address in amongst the usable addresses,
        or if the given CIDR is contained in this one."""
        if not isinstance(ip, (IPv4Address, CIDR)):
            if str(ip).find('/') == -1:
                ip = IPv4Address(ip)
            else:
                # Support for CIDR strings/objects, an idea of Nicola Novello.
                ip = CIDR(ip)
        if isinstance(ip, IPv4Address):
            if ip < self._first_ip or ip > self._last_ip:
                return False
        elif isinstance(ip, CIDR):
            # NOTE: manage /31 networks; 127.0.0.1/31 is considered to
            #       be included in 127.0.0.1/8.
            if ip._nm._ip_dec == 0xFFFFFFFEL \
                    and self._nm._ip_dec != 0xFFFFFFFEL:
                compare_to_first = self._net_ip._ip_dec
                compare_to_last = self._bc_ip._ip_dec
            else:
                compare_to_first = self._first_ip._ip_dec
                compare_to_last = self._last_ip._ip_dec
            if ip._first_ip._ip_dec < compare_to_first or \
                    ip._last_ip._ip_dec > compare_to_last:
                return False
        return True

    def __str__(self):
        """Print this CIDR address."""
        return self.get()

    def __repr__(self):
        """The representation string for this netmask."""
        return '<%s/%s CIDR>' % (str(self.get_ip()), str(self.get_netmask()))

    def __len__(self):
        """Return the number of usable IP address."""
        return self.get_ip_number()

    def __cmp__(self, other):
        """Compare two CIDR objects."""
        if not isinstance(other, self.__class__):
            other = self.__class__(other)
        # NOTE: the only really interesting result is 0 to test equality;
        #       in any other case: if this is a "wider" subnet, return 1;
        #       if they are of the same width, sort by IPs.
        if self._nm < other._nm:
            return -1
        elif self._nm > other._nm:
            return 1
        if self._ip < other._ip:
            return -1
        elif self._ip > other._ip:
            return 1
        return 0

    def __contains__(self, item):
        """Return true if the given address in amongst the usable addresses,
        or if the given CIDR is contained in this one."""
        return self.is_valid_ip(item)

    def __iter__(self):
        """Iterate over IPv4Address objects, one for every usable IP."""
        for i in xrange(0, self._ip_num):
                yield IPv4Address(self._first_ip_dec + i, notation=IP_DEC)

    cidr = property(get, set, doc='The represented CIDR.')
    ip = address = property(get_ip, set_ip, doc='The IP of this CIDR.')
    nm = netmask = property(get_netmask, set_netmask,
                            doc='The netmask of this CIDR.')
    first_ip = property(get_first_ip)
    last_ip = property(get_last_ip)
    network_ip = property(get_network_ip)
    broadcast_ip = property(get_broadcast_ip)
    ip_number = property(get_ip_number)


