<?php 
	$replace = array('</p>', '<br>', '<br/>', '<br />', '</li>', '</div>');

	$output = str_replace($replace, '<br style="mso-data-placement:same-cell">', $tbody);
	$output = strip_tags($output, '<tr><td><th><br><b><strong><i><em><a><img>');
	
	print $output; 
	
?>