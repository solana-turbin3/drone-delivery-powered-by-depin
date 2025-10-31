use anchor_lang::prelude::*;

use crate::state::*;

#[event]
pub struct DroneAvailableEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
}

#[event]
pub struct ReadyToFlyEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
}

#[event]
pub struct DroneArrivedEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
    pub status: DroneStatus,
}

#[event]
pub struct DroneUnAvailableEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
}

#[event]
pub struct DroneErrorEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
}

#[event]
pub struct LetterboxOpenEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
}

#[event]
pub struct LetterboxClosedEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
}