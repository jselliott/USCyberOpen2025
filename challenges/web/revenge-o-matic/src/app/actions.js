'use server'

const { approveSubmission } = require('../db/actions');

export async function approveBegging(formData) {

    const id = formData.get('id');

    if (!id) throw new Error('No id provided');

    approveSubmission(id);
}