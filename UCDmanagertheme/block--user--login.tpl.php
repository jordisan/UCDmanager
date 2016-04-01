  <?php

    $elements = drupal_get_form('user_login_block');

    /**
      do all your rendering stuff here
      drupal_render seems to add html to the elements array
      and instead of printing what is returned from drupal_render
      you can use the added html in ['#children'] elements of the arrays
      to build the form in the order you want.
    **/
	$elements['name']['#title'] = 'Username (existing)';
    $rendered = drupal_render($elements);

    // to see what you have to work with
    // print "<pre>ELEMENTS: " . print_r($elements,1) . "</pre>";

	$output = '<div class="block or">or</div>';
	$output .= '<div class="block"><div class="mainlink"><a class="userregister" href="/user/register" title="Create a new user account.">Create new account</a></div></div>';
	$output .= '<div class="block or">or</div>';
    $output .= '<form class="block" action="/' . // $elements['#action'] .
                              '" method="get' . // $elements['#method'] .
                              '" id="user-login-form' . //' . $elements['#id'] .
                              '" name="user-login-form' .
                              '" accept-charset="UTF-8"><div>';
	// $output .= '<h2>Existing user</h2>';
    $output .= $elements['name']['#children'];
    $output .= $elements['pass']['#children'];
    $output .= $elements['form_build_id']['#children'];
    $output .= $elements['form_id']['#children'];
    $output .= $elements['actions']['#children'];
	$output .= '<p><a class="reqnewpass" href="/user/password" title="Request new password via e-mail.">Request new password</a></p>';
	// $output .= $elements['links']['#children'];
    $output .= '</div></form>';

    // avoid flood
    $output .= '<script type="text/javascript">';
    $output .= "document.forms['user-login-form'].method = 'post';";
    $output .= "document.forms['user-login-form'].action = '/home?destination=my';";
    $output .= '</script>';

    print $output;
?>
