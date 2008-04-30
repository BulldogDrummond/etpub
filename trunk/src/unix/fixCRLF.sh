#!/bin/sh
find ../game -name "*.h" -exec ./dos2unix.pl {} \;
find ../game -name "*.c" -exec ./dos2unix.pl {} \;
find ../cgame -name "*.h" -exec ./dos2unix.pl {} \;
find ../cgame -name "*.c" -exec ./dos2unix.pl {} \;
find ../botai -name "*.c" -exec ./dos2unix.pl {} \;
find ../botai -name "*.h" -exec ./dos2unix.pl {} \;
find ../../Omnibot/Common -name "*.h" -exec ./dos2unix.pl {} \;
find ../../Omnibot/ET -name "*.h" -exec ./dos2unix.pl {} \;
