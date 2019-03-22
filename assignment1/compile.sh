#!/bin/bash
echo "Executing: sudo make -j2 CC=\"ccache gcc\""
# Make first compile call
sudo make -j2 CC="ccache gcc"

rc=$?;
if [[ $rc != 0 ]]; then
echo "Error occured executing: sudo make -j2 CC=\"ccache gcc\""
exit $rc;
else
echo "Completed!"
fi

echo "Executing: sudo make -j2 modules_install"
# Make second compile call
sudo make -j2 modules_install

rc=$?;
if [[ $rc != 0 ]]; then
echo "Error occured executing: sudo make -j2 modules_install"
exit $rc;
else
echo "Completed!"
fi

echo "Executing: sudo make -j2 install"
# Make last compile call
sudo make -j2 install

rc=$?;
if [[ $rc != 0 ]]; then
echo "Error occured executing: sudo make -j2 install"
exit $rc;
else
echo "Completed!"
fi
