#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "my_26robot_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__my_26robot_interfaces__msg__JointCommand() -> *const std::ffi::c_void;
}

#[link(name = "my_26robot_interfaces__rosidl_generator_c")]
extern "C" {
    fn my_26robot_interfaces__msg__JointCommand__init(msg: *mut JointCommand) -> bool;
    fn my_26robot_interfaces__msg__JointCommand__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<JointCommand>, size: usize) -> bool;
    fn my_26robot_interfaces__msg__JointCommand__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<JointCommand>);
    fn my_26robot_interfaces__msg__JointCommand__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<JointCommand>, out_seq: *mut rosidl_runtime_rs::Sequence<JointCommand>) -> bool;
}

// Corresponds to my_26robot_interfaces__msg__JointCommand
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct JointCommand {

    // This member is not documented.
    #[allow(missing_docs)]
    pub joint_angles: rosidl_runtime_rs::Sequence<f64>,

}



impl Default for JointCommand {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !my_26robot_interfaces__msg__JointCommand__init(&mut msg as *mut _) {
        panic!("Call to my_26robot_interfaces__msg__JointCommand__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for JointCommand {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__JointCommand__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__JointCommand__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__JointCommand__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for JointCommand {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for JointCommand where Self: Sized {
  const TYPE_NAME: &'static str = "my_26robot_interfaces/msg/JointCommand";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__my_26robot_interfaces__msg__JointCommand() }
  }
}


#[link(name = "my_26robot_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__my_26robot_interfaces__msg__JJointCommand() -> *const std::ffi::c_void;
}

#[link(name = "my_26robot_interfaces__rosidl_generator_c")]
extern "C" {
    fn my_26robot_interfaces__msg__JJointCommand__init(msg: *mut JJointCommand) -> bool;
    fn my_26robot_interfaces__msg__JJointCommand__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<JJointCommand>, size: usize) -> bool;
    fn my_26robot_interfaces__msg__JJointCommand__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<JJointCommand>);
    fn my_26robot_interfaces__msg__JJointCommand__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<JJointCommand>, out_seq: *mut rosidl_runtime_rs::Sequence<JJointCommand>) -> bool;
}

// Corresponds to my_26robot_interfaces__msg__JJointCommand
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct JJointCommand {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,


    // This member is not documented.
    #[allow(missing_docs)]
    pub joint_angles: [f64; 6],

}



impl Default for JJointCommand {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !my_26robot_interfaces__msg__JJointCommand__init(&mut msg as *mut _) {
        panic!("Call to my_26robot_interfaces__msg__JJointCommand__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for JJointCommand {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__JJointCommand__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__JJointCommand__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__JJointCommand__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for JJointCommand {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for JJointCommand where Self: Sized {
  const TYPE_NAME: &'static str = "my_26robot_interfaces/msg/JJointCommand";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__my_26robot_interfaces__msg__JJointCommand() }
  }
}


#[link(name = "my_26robot_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__my_26robot_interfaces__msg__PoseCommand() -> *const std::ffi::c_void;
}

#[link(name = "my_26robot_interfaces__rosidl_generator_c")]
extern "C" {
    fn my_26robot_interfaces__msg__PoseCommand__init(msg: *mut PoseCommand) -> bool;
    fn my_26robot_interfaces__msg__PoseCommand__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PoseCommand>, size: usize) -> bool;
    fn my_26robot_interfaces__msg__PoseCommand__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PoseCommand>);
    fn my_26robot_interfaces__msg__PoseCommand__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PoseCommand>, out_seq: *mut rosidl_runtime_rs::Sequence<PoseCommand>) -> bool;
}

// Corresponds to my_26robot_interfaces__msg__PoseCommand
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PoseCommand {

    // This member is not documented.
    #[allow(missing_docs)]
    pub x: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub y: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub z: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub roll: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub pitch: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub yaw: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub cartesian_path: bool,

}



impl Default for PoseCommand {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !my_26robot_interfaces__msg__PoseCommand__init(&mut msg as *mut _) {
        panic!("Call to my_26robot_interfaces__msg__PoseCommand__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PoseCommand {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__PoseCommand__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__PoseCommand__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_26robot_interfaces__msg__PoseCommand__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PoseCommand {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PoseCommand where Self: Sized {
  const TYPE_NAME: &'static str = "my_26robot_interfaces/msg/PoseCommand";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__my_26robot_interfaces__msg__PoseCommand() }
  }
}


