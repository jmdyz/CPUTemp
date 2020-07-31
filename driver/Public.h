/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_KMDFDriver1,
    0xbf9e7587,0x166d,0x44ca,0xbc,0x1e,0xe4,0x9a,0x48,0xf0,0x54,0x1a);
// {bf9e7587-166d-44ca-bc1e-e49a48f0541a}
