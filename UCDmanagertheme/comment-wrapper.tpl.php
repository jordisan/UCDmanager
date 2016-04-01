<section id="comments" class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <?php if ($content['comments'] && $node->type != 'forum'): ?>
    <?php print render($title_prefix); ?>
    <h2 class="comment-title title"><?php print t('COMMENTS to "') . $node->title . ('"'); ?></h2>
    <?php print render($title_suffix); ?>
  <?php endif; ?>

  <?php print render($content['comments']); ?>

  <?php if ($content['comment_form']): ?>
    <h2 class="comment-title title comment-form"><?php print t('NEW COMMENT to "') . $node->title . ('"'); ?></h2>
    <?php print render($content['comment_form']); ?>
  <?php endif; ?>
</section>
