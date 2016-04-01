<?php

define('UCDMANAGER_ID_NONMEMBER_ROLE',	1);
define('UCDMANAGER_ID_MEMBER_ROLE', 	2);
define('UCDMANAGER_ID_FULLMEMBER_ROLE',	4);
define('UCDMANAGER_ID_MANAGER_ROLE',	5);

/*
 * Permissions and group membership
 */

function ucdmanager_isSuperadmin($uid=null) {
	$user_account = null;
	if ( $uid ) {
		$user_account = user_load($uid);
	}
	// return user_access('bypass node access', $user_account);
	return user_access('administer nodes', $user_account);
}

function ucdmanager_getGroupManager($gid) {
	$uid = null;
	$group = og_load($gid);
	if ( $group ) {
		$entity = $group->getEntity();
		if ( $entity ) {
			$uid = $entity->uid;
		}
	}
	// error_log($gid . ' is managed by ' . $uid);
	return $uid;
}

function _isGroupManager($gid, $uid=null) {
	global $user;
	if ( $uid ) {
		$u = $uid;
	} else {
		$u = $user->uid;
	}
	// error_log($gid . ': ' . $u . '=?' . ucdmanager_getGroupManager($gid) );
	return ( $u == ucdmanager_getGroupManager($gid) );
}

function ucdmanager_isGroupMember($gid, $user=null) {
	if ( $user ) {
		$uid = $user->uid;
	} else {
		$uid = null;
	}
	return og_is_member($gid, 'user', $user) || _isGroupManager($gid, $uid);
}

function ucdmanager_isProjectManager($gid, $uid=null) {
	$is = false;
	if ( _isGroupManager($gid, $uid) ) {
		$is = true;
	} else {
		$roles = og_get_user_roles($gid, $uid, false);
		$is = in_array(UCDMANAGER_ID_MANAGER_ROLE, array_keys($roles));
	}
	return $is;
}

function ucdmanager_canCreateInGroup($gid, $type=null, $uid=null) {
	global $user;
	if ( $uid ) {
		$u = user_load($uid);
	} else {
		$u = $user;
	}	
	if ( ! ucdmanager_isGroupMember($gid, $u) ) {
		return false;
	} elseif ( ucdmanager_isProjectManager($gid, $uid) ) {
		return true;
	}

	$can = false;
	if ( !$type ) {
		$roles = og_get_user_roles($gid, $uid, false); // get roles different from non-member, member
		$can = !is_blank($roles);  // true if user has any other role
	} else {
		$roles = og_get_user_roles($gid, $uid, true);
		$permissions = og_role_permissions($roles);
		// $permissions has an array with the permissions for each role
		foreach ($permissions as $p) {
			$pcreate = 'create ' . str_replace('-', '_' , $type) . ' content';
			if ( array_key_exists($pcreate, $p) ) { // && !is_blank($p[$pcreate]) ) {
				// check the permission to create that specific content type
				$can = true;
				break;
			}
		}
	}
	return $can;
}

function ucdmanager_getRoleName($gid, $uid=null) {
	$UCDMANAGER_PROJECT_ROLENAMES = array (
		UCDMANAGER_ID_NONMEMBER_ROLE 	=> 'non-member',
		UCDMANAGER_ID_MEMBER_ROLE 		=> 'member (read-only)',
		UCDMANAGER_ID_FULLMEMBER_ROLE 	=> 'member',
		UCDMANAGER_ID_MANAGER_ROLE 		=> 'manager'	
	);

	$rolename = '?';
	if ( ucdmanager_isProjectManager($gid, $uid) ) {
		$rolename ='manager';
	} else {
		$roles = og_get_user_roles($gid, $uid, true);
		if ( $roles ) {
			$max = max(array_keys($roles));	
			if ( !is_blank($UCDMANAGER_PROJECT_ROLENAMES[$max]) ) {
				$rolename = $UCDMANAGER_PROJECT_ROLENAMES[$max];
			} else {
				$rolename = $roles[$max];
			}
		}
	}
	return $rolename;
}

?>
