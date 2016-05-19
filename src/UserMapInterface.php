<?php

/**
 * @file
 * Contains \Drupal\culturefeed\UserMapInterface.
 */

namespace Drupal\culturefeed;

use CultureFeed_User;

/**
 * The interface for mapping a culturefeed user.
 */
interface UserMapInterface {

  /**
   * Maps a culturefeed user to a user object.
   *
   * @param CultureFeed_User $user
   *   A Culturefeed user.
   *
   * @return mixed $user
   *   A user object.
   */
  public function get(CultureFeed_User $user);

  /**
   * Returns the culturefeed user id.
   *
   * @param int $drupal_id
   *   A drupal user id.
   *
   * @return string
   *   A culturefeed user id.
   */
  public function getCultureFeedId($drupal_id);

  /**
   * Returns the drupal user id.
   *
   * @param string $uitid
   *   A culturefeed user id.
   *
   * @return int
   *   A drupal user id.
   */
  public function getDrupalId($uitid);

}
